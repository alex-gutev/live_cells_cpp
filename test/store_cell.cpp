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

#include "computed.hpp"
#include "mutable_cell.hpp"
#include "store_cell.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(store_cell)

BOOST_AUTO_TEST_CASE(takes_argument_cell_value) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    BOOST_CHECK_EQUAL(store.value(), "hello");
}

BOOST_AUTO_TEST_CASE(takes_latest_argument_cell_value) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    a.value("bye");

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(store, observer);

    BOOST_CHECK_EQUAL(store.value(), "bye");
}

BOOST_AUTO_TEST_CASE(observers_notified_when_argument_changes) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(store, observer);

    a.value("bye");
    a.value("goodbye");

    BOOST_CHECK_EQUAL(observer->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(all_observers_notified_when_argument_changes) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    auto guard1 = with_observer(store, observer1);
    a.value("bye");

    auto guard2 = with_observer(store, observer2);
    a.value("goodbye");

    BOOST_CHECK_EQUAL(observer1->notify_count, 2);
    BOOST_CHECK_EQUAL(observer2->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observer_not_called_after_removal) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer = std::make_shared<simple_observer>();

    {
        auto guard = with_observer(store, observer);
        a.value("bye");
    }

    a.value("goodbye");

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(value_updated_when_observer_called) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer = std::make_shared<value_observer<std::string>>(store);
    auto guard = with_observer(store, observer);

    a.value("bye");

    BOOST_CHECK(observer->values == std::vector<std::string>({"bye"}));
}

BOOST_AUTO_TEST_CASE(previous_value_preserved_if_none_used) {
    auto a = live_cells::variable(0);
    auto evens = live_cells::computed(a, [] (auto a) {
        if ((a % 2) != 0) {
            live_cells::none();
        }

        return a;
    });

    auto store = live_cells::store(evens);

    auto observer = std::make_shared<value_observer<int>>(store);
    auto guard = with_observer(store, observer);

    a.value(1);
    a.value(2);
    a.value(3);
    a.value(4);
    a.value(5);

    BOOST_CHECK(observer->values == std::vector<int>({0, 2, 4}));
}

BOOST_AUTO_TEST_CASE(exception_in_init_handled) {
    auto a = live_cells::variable(0);
    auto cell = live_cells::computed(a, [] (auto a) {
        if (a == 0) {
            throw an_exception();
        }

        return a;
    });

    auto store = live_cells::store(a);

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(exception_in_init_reproduced_on_access_while_observed) {
    auto a = live_cells::variable(0);
    auto cell = live_cells::computed(a, [] (auto a) {
        if (a == 0) {
            throw an_exception();
        }

        return a;
    });

    auto store = live_cells::store(a);

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(store, observer);

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(compare_equal_if_same_argument_cell) {
    auto a = live_cells::variable(0);
    auto b = live_cells::computed(a, [] (auto a) {
        return a * a;
    });

    live_cells::observable_ref s1 = live_cells::store(b);
    live_cells::observable_ref s2 = live_cells::store(b);

    std::hash<live_cells::observable_ref> hash;

    BOOST_CHECK(s1 == s2);
    BOOST_CHECK(!(s1 != s2));
    BOOST_CHECK(hash(s1) == hash(s2));
}

BOOST_AUTO_TEST_CASE(compare_not_equal_if_different_argument_cell) {
    auto a = live_cells::variable(0);

    auto b1 = live_cells::computed(a, [] (auto a) {
        return a * a;
    });

    auto b2 = live_cells::computed(a, [] (auto a) {
        return a + a;
    });

    live_cells::observable_ref s1 = live_cells::store(b1);
    live_cells::observable_ref s2 = live_cells::store(b2);

    BOOST_CHECK(s1 != s2);
    BOOST_CHECK(!(s1 == s2));
}

BOOST_AUTO_TEST_CASE(manage_same_observers) {
    auto counter = std::make_shared<state_counter>();

    {
        auto a = test_managed_cell<int>(counter, 1);
        auto b = live_cells::variable(0);

        auto c = live_cells::computed(a, b, [] (auto a, auto b) {
            return a + b;
        });

        auto f = [=] {
            return live_cells::store(c);
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
    auto b = live_cells::variable(0);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto f = [=] {
        return live_cells::store(c);
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

BOOST_AUTO_TEST_SUITE_END()
