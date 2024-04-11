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
#include "mutable_cell.hpp"
#include "watcher.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(cell_watcher)

BOOST_AUTO_TEST_CASE(called_on_register) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    value_observer<int> obs(a);

    auto watch = live_cells::watch([&] {
        obs.values.push_back(a() + b());
    });

    obs.check_values({3});
}

BOOST_AUTO_TEST_CASE(called_when_cell_values_change) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    value_observer<int> obs(a);

    auto watch = live_cells::watch([&] {
        obs.values.push_back(a() + b());
    });

    a = 5;
    b = 10;

    obs.check_values({3, 7, 15});
}

BOOST_AUTO_TEST_CASE(called_when_cell_values_change_during_batch_update) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    value_observer<int> obs(a);

    auto watch = live_cells::watch([&] {
        obs.values.push_back(a() + b());
    });

    live_cells::batch([=] {
        a = 5;
        b = 10;
    });

    obs.check_values({3, 15});
}

BOOST_AUTO_TEST_CASE(called_when_cell_values_change_in_conditional_expression) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);
    auto select = live_cells::variable(true);

    value_observer<int> obs(a);

    auto watch = live_cells::watch([&] {
        if (select()) {
            obs.values.push_back(a());
        }
        else {
            obs.values.push_back(b());
        }
    });

    a = 3;
    select = false;
    b = 5;

    obs.check_values({1, 3, 2, 5});
}


BOOST_AUTO_TEST_CASE(not_called_after_stop) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    value_observer<int> obs(a);

    auto watch = live_cells::watch([&] {
        obs.values.push_back(a() + b());
    });

    a = 5;
    b = 10;
    watch->stop();

    b = 100;
    a = 30;

    obs.check_values({3, 7, 15});
}

BOOST_AUTO_TEST_CASE(init_called_when_cell_is_watched) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    auto watch = live_cells::watch([&] {
        cell();
        cell();
    });

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 1);
    BOOST_CHECK_EQUAL(counter->dispose_count, 0);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);
}

BOOST_AUTO_TEST_CASE(dispose_called_when_stop_called) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    auto watch = live_cells::watch([&] {
        cell();
        cell();
    });

    watch->stop();

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 1);
    BOOST_CHECK_EQUAL(counter->dispose_count, 1);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);
}

BOOST_AUTO_TEST_CASE(dispose_not_called_when_not_all_watchers_stopped) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    auto watch1 = live_cells::watch([&] {
        cell();
        cell();
    });

    auto watch2 = live_cells::watch([&] {
        cell();
        cell();
    });

    watch1->stop();

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 1);
    BOOST_CHECK_EQUAL(counter->dispose_count, 0);
    BOOST_CHECK_EQUAL(counter->dtor_count, 0);
}

BOOST_AUTO_TEST_CASE(watcher_stopped_in_destructor) {
    auto counter = std::make_shared<state_counter>();
    auto cell = test_managed_cell<int>(counter, 1);

    auto a = live_cells::variable(0);
    auto obs = value_observer<int>(a);

    {
        auto watch = live_cells::watch([&] {
            obs.values.push_back(a() + cell());
        });

        a = 2;
        a = 3;
    }

    a = 4;
    a = 5;

    obs.check_values({1, 3, 4});

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 1);
    BOOST_CHECK_EQUAL(counter->dispose_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
