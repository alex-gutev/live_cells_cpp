/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    auto cell = live_cells::value_cell(10);

    BOOST_CHECK_EQUAL(cell.value(), 10);
}

BOOST_AUTO_TEST_CASE(constructor_string) {
    auto cell = live_cells::value_cell(std::string("Hello World"));

    BOOST_CHECK_EQUAL(cell.value(), std::string("Hello World"));
}

BOOST_AUTO_TEST_CASE(constructor_bool) {
    auto cell1 = live_cells::value_cell(true);
    auto cell2 = live_cells::value_cell(false);

    BOOST_CHECK_EQUAL(cell1.value(), true);
    BOOST_CHECK_EQUAL(cell2.value(), false);
}

BOOST_AUTO_TEST_CASE(constructor_null) {
    auto cell = live_cells::value_cell<void*>(nullptr);

    BOOST_CHECK_EQUAL(cell.value(), nullptr);
}

// TODO: Add enum test cases

BOOST_AUTO_TEST_CASE(same_values_compare_equal) {
    live_cells::cell cell1(live_cells::value_cell(3));
    live_cells::cell cell2(live_cells::value_cell(3));

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(cell1 == cell2);
    BOOST_CHECK(!(cell1 != cell2));
    BOOST_CHECK(hash(cell1) == hash(cell2));
}

BOOST_AUTO_TEST_CASE(different_value_compare_not_equal) {
    live_cells::cell cell1(live_cells::value_cell(1));
    live_cells::cell cell2(live_cells::value_cell(2));

    BOOST_CHECK(cell1 != cell2);
    BOOST_CHECK(!(cell1 == cell2));
}

BOOST_AUTO_TEST_SUITE_END()
