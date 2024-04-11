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

#include "mutable_cell.hpp"
#include "test_util.hpp"

BOOST_AUTO_TEST_SUITE(mutable_cell)

BOOST_AUTO_TEST_CASE(keeps_initial_value) {
    auto cell = live_cells::variable(15);
    BOOST_CHECK_EQUAL(cell.value(), 15);
}

BOOST_AUTO_TEST_CASE(set_value_changes_value) {
    auto cell = live_cells::variable(15);
    cell.value(23);

    BOOST_CHECK_EQUAL(cell.value(), 23);
}

BOOST_AUTO_TEST_CASE(keeps_latest_set_value) {
    auto cell = live_cells::variable(15);
    cell = 23;
    cell = 101;

    BOOST_CHECK_EQUAL(cell.value(), 101);
}

BOOST_AUTO_TEST_CASE(set_value_calls_observers) {
    auto cell = live_cells::variable(10);
    auto observer = std::make_shared<simple_observer>();

    auto guard = with_observer(cell, observer);

    cell = 23;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(set_value_twice_calls_observers_twice) {
    auto cell = live_cells::variable(10);
    auto observer = std::make_shared<simple_observer>();

    auto guard = with_observer(cell, observer);

    cell = 23;
    cell = 101;

    BOOST_CHECK_EQUAL(observer->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(observer_not_called_after_removal) {
    auto cell = live_cells::variable(10);
    auto observer = std::make_shared<simple_observer>();

    {
        auto guard = with_observer(cell, observer);
        cell = 23;
    }

    cell = 101;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observer_not_called_if_new_value_equals_old) {
    auto cell = live_cells::variable(56);
    auto observer = std::make_shared<simple_observer>();

    auto guard = with_observer(cell, observer);
    cell = 56;

    BOOST_CHECK_EQUAL(observer->notify_count, 0);
}

BOOST_AUTO_TEST_CASE(all_observers_called) {
    auto cell = live_cells::variable(3);

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    auto guard1 = with_observer(cell, observer1);
    cell = 5;

    auto guard2 = with_observer(cell, observer2);
    cell = 8;
    cell = 12;

    BOOST_CHECK_EQUAL(observer1->notify_count, 3);
    BOOST_CHECK_EQUAL(observer2->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(value_updated_when_observer_called) {
    auto cell = live_cells::variable<std::string>("hello");
    auto observer = std::make_shared<value_observer<std::string>>(cell);

    auto guard = with_observer(cell, observer);

    cell = "bye";

    BOOST_CHECK(observer->values == std::vector<std::string>({"bye"}));
}

BOOST_AUTO_TEST_CASE(batch_updates) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(0);

    auto observer1 = std::make_shared<value_observer<int>>(a);
    auto observer2 = std::make_shared<value_observer<int>>(b);

    auto guard1 = with_observer(a, observer1);
    auto guard2 = with_observer(b, observer2);

    live_cells::batch([&] {
        a = 1;
        b = 2;
    });

    observer1->check_values({1});
    observer2->check_values({2});
}

BOOST_AUTO_TEST_SUITE_END()
