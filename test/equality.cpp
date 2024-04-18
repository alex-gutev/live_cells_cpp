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
#include "equality.hpp"

#include "test_util.hpp"


BOOST_AUTO_TEST_SUITE(equality_comparison)

BOOST_AUTO_TEST_CASE(constant_cells_equal_if_equal_values) {
    auto a = live_cells::value(1);
    auto b = live_cells::value(1);

    auto eq1 = a == b;
    auto eq2 = a == 1;
    auto eq3 = 1 == b;

    BOOST_CHECK(eq1.value());
    BOOST_CHECK(eq2.value());
    BOOST_CHECK(eq3.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_not_equal_if_unequal_values) {
    auto a = live_cells::value(1);
    auto b = live_cells::value(2);

    auto eq1 = a == b;
    auto eq2 = a == 2;
    auto eq3 = 1 == b;

    BOOST_CHECK(!eq1.value());
    BOOST_CHECK(!eq2.value());
    BOOST_CHECK(!eq3.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_unequal_if_unequal_values) {
    auto a = live_cells::value(3);
    auto b = live_cells::value(4);

    auto neq1 = a != b;
    auto neq2 = a != 4;
    auto neq3 = 3 != b;

    BOOST_CHECK(neq1.value());
    BOOST_CHECK(neq2.value());
    BOOST_CHECK(neq3.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_not_unequal_if_equal_values) {
    auto a = live_cells::value(3);
    auto b = live_cells::value(3);

    auto neq1 = a != b;
    auto neq2 = a != 3;
    auto neq3 = 3 != b;

    BOOST_CHECK(!neq1.value());
    BOOST_CHECK(!neq2.value());
    BOOST_CHECK(!neq3.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_recomputed_when_1st_arg_changes) {
    auto a = live_cells::variable(3);
    auto eq = a == 4;

    BOOST_CHECK(!eq.value());

    a = 4;
    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_recomputed_when_2nd_arg_changes) {
    auto b = live_cells::variable(4);

    auto eq = 3 == b;

    BOOST_CHECK(!eq.value());

    b = 3;
    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_observers_notified_on_1st_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(eq, observer);

    a = 4;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(equality_cell_observers_notified_on_2nd_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(eq, observer);

    b = 3;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(inequality_cell_recomputed_when_1st_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != 4;
    BOOST_CHECK(neq.value());

    a = 4;
    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(inequality_cell_recomputed_when_2nd_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = 3 != b;
    BOOST_CHECK(neq.value());

    b = 3;
    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(inequality_cell_observers_notified_on_1st_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(neq, observer);

    a = 4;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(inequality_cell_observers_notified_on_2nd_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(neq, observer);

    b = 3;

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

BOOST_AUTO_TEST_CASE(equality_cells_compare_equal_if_same_arguments_with_literal_values) {
    auto a = live_cells::value(1);
    auto b = live_cells::value(2);

    live_cells::cell eq1(a == 2);
    live_cells::cell eq2(1 == b);

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(eq1 == eq2);
    BOOST_CHECK(!(eq1 != eq2));
    BOOST_CHECK(hash(eq1) == hash(eq2));
}

BOOST_AUTO_TEST_CASE(equality_cells_compare_not_equal_if_different_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::cell eq1(a == b);
    live_cells::cell eq2(a == live_cells::value(2));
    live_cells::cell eq3(live_cells::value(2) == b);

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

BOOST_AUTO_TEST_CASE(inequality_cells_compare_equal_if_same_arguments_with_literal_values) {
    auto a = live_cells::value(1);
    auto b = live_cells::value(2);

    live_cells::cell neq1(a != 2);
    live_cells::cell neq2(1 != b);

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(neq1 == neq2);
    BOOST_CHECK(!(neq1 != neq2));
    BOOST_CHECK(hash(neq1) == hash(neq2));
}

BOOST_AUTO_TEST_CASE(inequality_cells_compare_not_equal_if_different_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::cell neq1(a != b);
    live_cells::cell neq2(a != live_cells::value(2));
    live_cells::cell neq3(live_cells::value(2) != b);

    BOOST_CHECK(neq1 != neq2);
    BOOST_CHECK(neq1 != neq3);
    BOOST_CHECK(!(neq1 == neq2));
}

BOOST_AUTO_TEST_SUITE_END()
