/*
 * live_cells_cpp
 * Copyright (C) 2025  Alexander Gutev <alex.gutev@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You
 * may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#define BOOST_TEST_DYN_LINK

#include <string>
#include <memory>
#include <vector>
#include <exception>

#include <boost/test/unit_test.hpp>

#include "computed.hpp"
#include "mutable_cell.hpp"
#include "maybe_cell.hpp"
#include "test_util.hpp"

BOOST_AUTO_TEST_SUITE(maybe_cell)

BOOST_AUTO_TEST_CASE(passes_through_cell_value) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed([=] {
        return a() + 1;
    });

    auto m = live_cells::maybe_cell(b);

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(m, observer);

    BOOST_CHECK(m.value() == live_cells::maybe(2));

    a = 2;
    BOOST_CHECK(m.value() == live_cells::maybe(3));
}

BOOST_AUTO_TEST_CASE(passes_through_mutable_cell_value) {
    auto a = live_cells::variable(1);

    auto m = live_cells::maybe_cell(a);

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(m, observer);

    BOOST_CHECK(m.value() == live_cells::maybe(1));

    a = 2;
    BOOST_CHECK(m.value() == live_cells::maybe(2));
}

BOOST_AUTO_TEST_CASE(passes_through_cell_exception) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed([=] {
        if ((a() % 2))
            throw an_exception();

        return a();
    });

    auto m = b | live_cells::ops::maybe();

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(m, observer);

    BOOST_CHECK_THROW(m.value().unwrap(), an_exception);

    a = 4;
    BOOST_CHECK(m.value().unwrap() == 4);
}

BOOST_AUTO_TEST_CASE(passes_through_mutable_cell_exception) {
    auto a = live_cells::variable(1);

    auto b = live_cells::mutable_computed([=] {
        if ((a() % 2))
            throw an_exception();

        return a();
    }, [] (auto v) {});

    auto m = b | live_cells::ops::maybe();

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(m, observer);

    BOOST_CHECK_THROW(m.value().unwrap(), an_exception);

    a = 4;
    BOOST_CHECK(m.value().unwrap() == 4);
}


BOOST_AUTO_TEST_CASE(mutable_maybe_cell) {
    auto a = live_cells::variable(1);
    auto m = a | live_cells::ops::maybe();

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(m, observer);

    m = live_cells::maybe(2);
    BOOST_CHECK(m.value().unwrap() == 2);

    m = live_cells::maybe_wrap([] {
        throw an_exception();
        return 1;
    });

    BOOST_CHECK_THROW(m.value().unwrap(), an_exception);

    m = live_cells::maybe(5);
    BOOST_CHECK(m.value().unwrap() == 5);
}

BOOST_AUTO_TEST_CASE(compare_equal_if_same_argument_cell) {
    auto a = live_cells::variable(1);

    auto m1 = a | live_cells::ops::maybe();
    auto m2 = a | live_cells::ops::maybe();

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(m1 == m2);
    BOOST_CHECK(!(m1 != m2));
    BOOST_CHECK(hash(m1) == hash(m2));
}

BOOST_AUTO_TEST_CASE(compare_not_equal_if_different_argument_cell) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(1);

    auto m1 = a | live_cells::ops::maybe();
    auto m2 = b | live_cells::ops::maybe();

    BOOST_CHECK(m1 != m2);
    BOOST_CHECK(!(m1 == m2));
}

BOOST_AUTO_TEST_SUITE_END()
