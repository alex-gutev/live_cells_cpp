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

    a = "bye";

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(store, observer);

    BOOST_CHECK_EQUAL(store.value(), "bye");
}

BOOST_AUTO_TEST_CASE(observers_notified_when_argument_changes) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(store, observer);

    a = "bye";
    a = "goodbye";

    BOOST_CHECK_EQUAL(observer->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(all_observers_notified_when_argument_changes) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    auto guard1 = with_observer(store, observer1);
    a = "bye";

    auto guard2 = with_observer(store, observer2);
    a = "goodbye";

    BOOST_CHECK_EQUAL(observer1->notify_count, 2);
    BOOST_CHECK_EQUAL(observer2->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observer_not_called_after_removal) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer = std::make_shared<simple_observer>();

    {
        auto guard = with_observer(store, observer);
        a = "bye";
    }

    a = "goodbye";

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(value_updated_when_observer_called) {
    auto a = live_cells::variable<std::string>("hello");
    auto store = live_cells::store(a);

    auto observer = std::make_shared<value_observer<std::string>>(store);
    auto guard = with_observer(store, observer);

    a = "bye";

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

    a = 1;
    a = 2;
    a = 3;
    a = 4;
    a = 5;

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

    live_cells::cell s1 = live_cells::store(b);
    live_cells::cell s2 = live_cells::store(b);

    std::hash<live_cells::cell> hash;

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

    live_cells::cell s1 = live_cells::store(b1);
    live_cells::cell s2 = live_cells::store(b2);

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
