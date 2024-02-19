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
#include "mutable_cell.hpp"
#include "numeric.hpp"

#include "test_util.hpp"


BOOST_AUTO_TEST_SUITE(numeric_operators)

BOOST_AUTO_TEST_CASE(additon) {
    auto a = live_cells::value_cell(5);
    auto b = live_cells::value_cell(6);

    auto c = a + b;

    BOOST_CHECK_EQUAL(c.value(), 11);
}

BOOST_AUTO_TEST_CASE(subtraction) {
    auto a = live_cells::value_cell(13);
    auto b = live_cells::value_cell(20);

    auto c = a - b;

    BOOST_CHECK_EQUAL(c.value(), -7);
}

BOOST_AUTO_TEST_CASE(multiplication) {
    auto a = live_cells::value_cell(10);
    auto b = live_cells::value_cell(8);

    auto c = a * b;

    BOOST_CHECK_EQUAL(c.value(), 80);
}

BOOST_AUTO_TEST_CASE(division) {
    auto a = live_cells::value_cell(8);
    auto b = live_cells::value_cell(4);

    auto c = a / b;

    BOOST_CHECK_EQUAL(c.value(), 2);
}

BOOST_AUTO_TEST_CASE(modulu) {
    auto a = live_cells::value_cell(17);
    auto b = live_cells::value_cell(3);

    auto c = a % b;

    BOOST_CHECK_EQUAL(c.value(), 2);
}

BOOST_AUTO_TEST_CASE(less_than) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(8);

    auto lt = a < b;
    auto gt = b < a;
    auto eq = a < a;

    BOOST_CHECK(lt.value());
    BOOST_CHECK(!gt.value());
    BOOST_CHECK(!eq.value());
}

BOOST_AUTO_TEST_CASE(less_than_or_equal) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(8);

    auto lt = a <= b;
    auto gt = b <= a;
    auto eq = a <= a;

    BOOST_CHECK(lt.value());
    BOOST_CHECK(!gt.value());
    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(greater_than) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(8);

    auto gt = a > b;
    auto lt = b > a;
    auto eq = a > a;

    BOOST_CHECK(!gt.value());
    BOOST_CHECK(lt.value());
    BOOST_CHECK(!eq.value());
}

BOOST_AUTO_TEST_CASE(greater_than_or_equal) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(8);

    auto gt = a >= b;
    auto lt = b >= a;
    auto eq = a >= a;

    BOOST_CHECK(!gt.value());
    BOOST_CHECK(lt.value());
    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_SUITE_END()
