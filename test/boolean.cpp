/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
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

#include "constant_cell.hpp"
#include "mutable_cell.hpp"
#include "boolean.hpp"

#include "test_util.hpp"


BOOST_AUTO_TEST_SUITE(numeric_operators)

BOOST_AUTO_TEST_CASE(logical_and) {
    auto a = live_cells::variable(true);
    auto b = live_cells::variable(true);

    auto c = a && b;

    BOOST_CHECK(c.value());

    a.value(false);
    BOOST_CHECK(!c.value());

    b.value(false);
    BOOST_CHECK(!c.value());

    a.value(true);
    BOOST_CHECK(!c.value());

    b.value(true);
    BOOST_CHECK(c.value());
}

BOOST_AUTO_TEST_CASE(logical_or) {
    auto a = live_cells::variable(true);
    auto b = live_cells::variable(true);

    auto c = a || b;

    BOOST_CHECK(c.value());

    a.value(false);
    BOOST_CHECK(c.value());

    b.value(false);
    BOOST_CHECK(!c.value());

    a.value(true);
    BOOST_CHECK(c.value());

    b.value(true);
    BOOST_CHECK(c.value());
}

BOOST_AUTO_TEST_CASE(logical_not) {
    auto a = live_cells::variable(true);
    auto b = !a;

    BOOST_CHECK(!(b.value()));

    a.value(false);
    BOOST_CHECK(b.value());
}

BOOST_AUTO_TEST_CASE(select_with_if_false) {
    auto a = live_cells::value<std::string>("true");
    auto b = live_cells::variable<std::string>("false");

    auto cond = live_cells::variable(true);
    auto select = live_cells::select(cond, a, b);

    auto observer = std::make_shared<value_observer<std::string>>(select);
    auto guard = with_observer(select, observer);

    BOOST_CHECK_EQUAL(select.value(), "true");

    cond.value(false);
    b.value("else");
    cond.value(true);

    BOOST_CHECK(observer->values == std::vector<std::string>({"false", "else", "true"}));

}

BOOST_AUTO_TEST_CASE(select_without_if_false) {
    auto a = live_cells::variable<std::string>("true");

    auto cond = live_cells::variable(true);
    auto select = live_cells::select(cond, a);

    auto guard = with_observer(select, std::make_shared<simple_observer>());

    BOOST_CHECK_EQUAL(select.value(), "true");

    cond.value(false);
    BOOST_CHECK_EQUAL(select.value(), "true");

    a.value("then");
    BOOST_CHECK_EQUAL(select.value(), "true");

    cond.value(true);
    BOOST_CHECK_EQUAL(select.value(), "then");

    a.value("when");
    BOOST_CHECK_EQUAL(select.value(), "when");
}

BOOST_AUTO_TEST_SUITE_END()
