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
#include "computed.hpp"
#include "previous_value_cell.hpp"
#include "numeric.hpp"
#include "watcher.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(previous_value_cell)

BOOST_AUTO_TEST_CASE(hold_exception_on_init) {
    auto a = live_cells::variable(0);
    auto prev = a | live_cells::ops::previous;

    BOOST_CHECK_THROW(prev.value(), live_cells::uninitialized_cell_error);
}

BOOST_AUTO_TEST_CASE(holds_previous_value_when_set_once) {
    auto a = live_cells::variable(5);
    auto prev = a | live_cells::ops::previous;

    auto guard = observer_guard(prev, std::make_shared<simple_observer>());

    a = 10;

    BOOST_CHECK_EQUAL(prev.value(), 5);
}

BOOST_AUTO_TEST_CASE(holds_previous_value_when_set_multiple_times) {
    auto a = live_cells::variable(5);
    auto prev = a | live_cells::ops::previous;

    auto observer = std::make_shared<value_observer<int>>(prev);
    auto guard = observer_guard(prev, observer);

    a = 10;
    a = 8;
    a = 32;
    a = 40;

    observer->check_values({5, 10, 8, 32});

    BOOST_CHECK_EQUAL(prev.value(), 32);
}

BOOST_AUTO_TEST_CASE(exceptions_reproduced) {
    auto a = live_cells::variable(5);
    auto b = live_cells::computed([=] {
        if (a() < 0) {
            throw an_exception();
        }

        return a();
    });

    auto prev = b | live_cells::ops::previous;

    auto observer = std::make_shared<simple_observer>();
    auto guard = observer_guard(prev, observer);

    a = 10;
    BOOST_CHECK_EQUAL(prev.value(), 5);

    a = -3;
    BOOST_CHECK_EQUAL(prev.value(), 10);

    a = 4;
    BOOST_CHECK_THROW(prev.value(), an_exception);

    a = -9;
    BOOST_CHECK_EQUAL(prev.value(), 4);
}

BOOST_AUTO_TEST_CASE(compare_equal_if_same_argument_cell) {
    using live_cells::ops::previous;

    auto a = live_cells::variable(0);

    live_cells::cell p1 = a | previous;
    live_cells::cell p2 = a | previous;

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(p1 == p2);
    BOOST_CHECK(!(p1 != p2));
    BOOST_CHECK(hash(p1) == hash(p2));
}

BOOST_AUTO_TEST_CASE(compare_not_equal_if_different_argument_cell) {
    using live_cells::ops::previous;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(0);

    live_cells::cell p1 = a | previous;
    live_cells::cell p2 = b | previous;

    BOOST_CHECK(p1 == p1);
    BOOST_CHECK(p1 != p2);
    BOOST_CHECK(!(p1 == p2));
}

BOOST_AUTO_TEST_CASE(manage_same_observers) {
    auto counter = std::make_shared<state_counter>();

    {
        auto a = test_managed_cell<int>(counter, 1);
        auto b = live_cells::variable(0);
        auto c = a + b;

        auto f = [=] {
            return live_cells::previous(c);
        };

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 0);
        BOOST_CHECK_EQUAL(counter->dispose_count, 0);

        auto observer = std::make_shared<simple_observer>();

        f().add_observer(observer);
        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);

        f().remove_observer(observer);
        BOOST_CHECK_EQUAL(counter->dispose_count, 1);
    }

    BOOST_CHECK_EQUAL(counter->dtor_count, 1);
}

BOOST_AUTO_TEST_CASE(state_reinitialized_on_add_observer_post_pause) {
    auto counter = std::make_shared<state_counter>();

    auto a = test_managed_cell<int>(counter, 1);
    auto b = live_cells::variable(0);
    auto c = a + b;

    auto f = [=] {
        return live_cells::previous(c);
    };

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 0);
    BOOST_CHECK_EQUAL(counter->dispose_count, 0);

    auto observer = std::make_shared<simple_observer>();

    {
        auto cell = f();
        auto guard = with_observer(cell, observer);

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);
    }

    BOOST_CHECK_EQUAL(counter->dispose_count, 1);

    {
        auto cell = f();
        auto guard = with_observer(cell, observer);

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 2);
    }

    BOOST_CHECK_EQUAL(counter->dispose_count, 2);
}

BOOST_AUTO_TEST_CASE(does_not_notify_observers_when_value_unchanged) {
    auto a = live_cells::variable(std::vector({0, 0, 0}));
    auto b = live_cells::computed(live_cells::changes_only, [=] {
        return a()[1];
    });

    auto prev = b | live_cells::ops::previous;

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(prev());
    });

    a = {1, 2, 3};
    a = {4, 2, 6};
    a = {7, 8, 9};
    a = {10, 8, 11};
    a = {12, 13, 14};

    BOOST_CHECK(values == std::vector({0, 2, 8}));
}

BOOST_AUTO_TEST_SUITE_END()
