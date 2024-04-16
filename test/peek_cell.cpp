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
#include "peek_cell.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(peek_cell)

BOOST_AUTO_TEST_CASE(value_equals_argument_cell) {
    auto a = live_cells::variable(0);
    auto p = a | live_cells::ops::peek;

    BOOST_CHECK_EQUAL(p.value(), 0);

    a = 2;
    BOOST_CHECK_EQUAL(p.value(), 2);
}

BOOST_AUTO_TEST_CASE(does_not_notify_observers) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    auto sum = live_cells::computed([=] {
        return live_cells::peek(a)() + b();
    });

    auto observer = std::make_shared<value_observer<int>>(sum);
    auto guard = with_observer(sum, observer);

    a = 1;
    a = 2;
    a = 3;
    b = 5;
    b = 10;
    a = 2;
    b = 13;

    observer->check_values({8, 13, 15});
}

BOOST_AUTO_TEST_CASE(compare_equal_if_same_argument_cell) {
    auto a = live_cells::variable(0);

    live_cells::cell p1 = a | live_cells::ops::peek;
    live_cells::cell p2 = a | live_cells::ops::peek;

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(p1 == p2);
    BOOST_CHECK(!(p1 != p2));
    BOOST_CHECK(hash(p1) == hash(p2));
}

BOOST_AUTO_TEST_CASE(compare_not_equal_if_different_argument_cells) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(0);

    live_cells::cell p1 = a | live_cells::ops::peek;
    live_cells::cell p2 = b | live_cells::ops::peek;

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(p1 != p2);
    BOOST_CHECK(!(p1 == p2));
}

BOOST_AUTO_TEST_CASE(manage_same_observers) {
    auto counter = std::make_shared<state_counter>();

    {
        auto a = test_managed_cell<int>(counter, 1);

        auto f = [=] {
            return a | live_cells::ops::peek;
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

BOOST_AUTO_TEST_CASE(state_reinitialized_on_add_observer_post_dispose) {
    auto counter = std::make_shared<state_counter>();

    auto a = test_managed_cell<int>(counter, 1);

    auto f = [=] {
        return live_cells::peek(a);
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

BOOST_AUTO_TEST_CASE(correct_observer_removed) {
    auto counter = std::make_shared<state_counter>();

    auto a = test_managed_cell<int>(counter, 1);

    auto f = [=] {
        return live_cells::peek(a);
    };

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 0);
    BOOST_CHECK_EQUAL(counter->dispose_count, 0);

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    {
        auto cell = f();
        auto guard = with_observer(cell, observer1);

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);

        f().remove_observer(observer2);
        BOOST_CHECK_EQUAL(counter->dispose_count, 0);
    }

    BOOST_CHECK_EQUAL(counter->dispose_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
