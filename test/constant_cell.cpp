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

BOOST_AUTO_TEST_SUITE(constant_cell)

BOOST_AUTO_TEST_CASE(constructor_integer) {
    auto cell = live_cells::value(10);

    BOOST_CHECK_EQUAL(cell.value(), 10);
}

BOOST_AUTO_TEST_CASE(constructor_string) {
    auto cell = live_cells::value(std::string("Hello World"));

    BOOST_CHECK_EQUAL(cell.value(), std::string("Hello World"));
}

BOOST_AUTO_TEST_CASE(constructor_bool) {
    auto cell1 = live_cells::value(true);
    auto cell2 = live_cells::value(false);

    BOOST_CHECK_EQUAL(cell1.value(), true);
    BOOST_CHECK_EQUAL(cell2.value(), false);
}

BOOST_AUTO_TEST_CASE(constructor_null) {
    auto cell = live_cells::value<void*>(nullptr);

    BOOST_CHECK_EQUAL(cell.value(), nullptr);
}

// TODO: Add enum test cases

BOOST_AUTO_TEST_CASE(same_values_compare_equal) {
    live_cells::cell cell1(live_cells::value(3));
    live_cells::cell cell2(live_cells::value(3));

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(cell1 == cell2);
    BOOST_CHECK(!(cell1 != cell2));
    BOOST_CHECK(hash(cell1) == hash(cell2));
}

BOOST_AUTO_TEST_CASE(different_value_compare_not_equal) {
    live_cells::cell cell1(live_cells::value(1));
    live_cells::cell cell2(live_cells::value(2));

    BOOST_CHECK(cell1 != cell2);
    BOOST_CHECK(!(cell1 == cell2));
}

BOOST_AUTO_TEST_SUITE_END()
