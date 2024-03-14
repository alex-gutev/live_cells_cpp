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
#include "equality.hpp"

#include "test_util.hpp"


BOOST_AUTO_TEST_SUITE(equality_comparison)

BOOST_AUTO_TEST_CASE(constant_cells_equal_if_equal_values) {
    auto a = live_cells::value_cell(1);
    auto b = live_cells::value_cell(1);

    auto eq = a == b;

    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_not_equal_if_unequal_values) {
    auto a = live_cells::value_cell(1);
    auto b = live_cells::value_cell(2);

    auto eq = a == b;

    BOOST_CHECK(!eq.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_unequal_if_unequal_values) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(4);

    auto neq = a != b;

    BOOST_CHECK(neq.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_not_unequal_if_equal_values) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(3);

    auto neq = a != b;

    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_recomputed_when_1st_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    a.value(4);

    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_recomputed_when_2nd_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    b.value(3);

    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_observers_notified_on_1st_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(eq, observer);

    a.value(4);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(equality_cell_observers_notified_on_2nd_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(eq, observer);

    b.value(3);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(inequality_cell_recomputed_when_1st_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    a.value(4);

    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(inequality_cell_recomputed_when_2nd_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    b.value(3);

    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(inequality_cell_observers_notified_on_1st_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(neq, observer);

    a.value(4);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(inequality_cell_observers_notified_on_2nd_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(neq, observer);

    b.value(3);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(equality_cells_compare_equal_if_same_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::cell eq1(a == b);
    live_cells::cell eq2(a == b);

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(eq1 == eq2);
    BOOST_CHECK(!(eq1 != eq2));
    BOOST_CHECK(hash(eq1) == hash(eq2));
}

BOOST_AUTO_TEST_CASE(equality_cells_compare_not_equal_if_different_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::cell eq1(a == b);
    live_cells::cell eq2(a == live_cells::value_cell(2));
    live_cells::cell eq3(live_cells::value_cell(2) == b);

    BOOST_CHECK(eq1 != eq2);
    BOOST_CHECK(eq1 != eq3);
    BOOST_CHECK(!(eq1 == eq2));
}

BOOST_AUTO_TEST_CASE(inequality_cells_compare_equal_if_same_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::cell neq1(a != b);
    live_cells::cell neq2(a != b);

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(neq1 == neq2);
    BOOST_CHECK(!(neq1 != neq2));
    BOOST_CHECK(hash(neq1) == hash(neq2));
}

BOOST_AUTO_TEST_CASE(inequality_cells_compare_not_equal_if_different_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::cell neq1(a != b);
    live_cells::cell neq2(a != live_cells::value_cell(2));
    live_cells::cell neq3(live_cells::value_cell(2) != b);

    BOOST_CHECK(neq1 != neq2);
    BOOST_CHECK(neq1 != neq3);
    BOOST_CHECK(!(neq1 == neq2));
}

BOOST_AUTO_TEST_SUITE_END()
