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

    a.value(5);
    b.value(10);

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
        a.value(5);
        b.value(10);
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

    a.value(3);
    select.value(false);
    b.value(5);

    obs.check_values({1, 3, 2, 5});
}


BOOST_AUTO_TEST_CASE(not_called_after_stop) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    value_observer<int> obs(a);

    auto watch = live_cells::watch([&] {
        obs.values.push_back(a() + b());
    });

    a.value(5);
    b.value(10);
    watch->stop();

    b.value(100);
    a.value(30);

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

        a.value(2);
        a.value(3);
    }

    a.value(4);
    a.value(5);

    obs.check_values({1, 3, 4});

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 1);
    BOOST_CHECK_EQUAL(counter->dispose_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
