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

#include "keys.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(init_and_cleanup)

BOOST_AUTO_TEST_CASE(init_not_called_before_adding_observer) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 0);
    BOOST_CHECK_EQUAL(counter->dispose_count, 0);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);
}

BOOST_AUTO_TEST_CASE(init_called_on_adding_first_observer) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    auto guard1 = with_observer(cell, std::make_shared<simple_observer>());
    auto guard2 = with_observer(cell, std::make_shared<simple_observer>());

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 1);
    BOOST_CHECK_EQUAL(counter->dispose_count, 0);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);
}

BOOST_AUTO_TEST_CASE(pause_not_called_before_removing_all_observers) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    auto guard1 = with_observer(cell, std::make_shared<simple_observer>());

    {
        auto guard2 = with_observer(cell, std::make_shared<simple_observer>());

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);
    }

    BOOST_CHECK_EQUAL(counter->dispose_count, 0);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);
}

BOOST_AUTO_TEST_CASE(pause_called_when_all_observers_removed) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    {
        auto guard1 = with_observer(cell, std::make_shared<simple_observer>());
        auto guard2 = with_observer(cell, std::make_shared<simple_observer>());

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);
    }

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 1);
    BOOST_CHECK_EQUAL(counter->dispose_count, 1);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);
}

BOOST_AUTO_TEST_CASE(init_called_gain_when_adding_observer_after_pause) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    auto observer = std::make_shared<simple_observer>();

    {
        auto guard1 = with_observer(cell, observer);
        auto guard2 = with_observer(cell, std::make_shared<simple_observer>());

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);
    }

    {
        auto guard1 = with_observer(cell, observer);

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 2);
        BOOST_CHECK_EQUAL(counter->dispose_count, 1);
        BOOST_CHECK_EQUAL(counter->dtor_count, 0);
    }
}

BOOST_AUTO_TEST_CASE(cell_state_destroyed_when_all_cells_destroyed) {
    auto counter = std::make_shared<state_counter>();

    {
        auto cell1 = test_managed_cell<int>(counter, 1);

        {
            auto cell2 = cell1;

            BOOST_CHECK_EQUAL(counter->ctor_count, 1);
            BOOST_CHECK_EQUAL(counter->dtor_count, 0);
        }

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->dtor_count, 0);
    }

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->dtor_count, 1);
}

BOOST_AUTO_TEST_CASE(cell_state_not_destroyed_before_all_cells_destroyed) {
    auto counter = std::make_shared<state_counter>();

    std::shared_ptr<test_managed_cell<int>> ref;

    {
        auto cell1 = test_managed_cell<int>(counter, 1);

        ref = std::make_shared<test_managed_cell<int>>(cell1);


        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->dtor_count, 0);
    }

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);

    ref = nullptr;

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->dtor_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
