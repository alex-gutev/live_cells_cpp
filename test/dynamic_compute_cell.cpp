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
#include "computed.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(dynamic_compute_cell)

BOOST_AUTO_TEST_CASE(applied_on_constant_cell_value) {
    auto a = live_cells::value(1);
    auto b = live_cells::computed([=] {
        return a() + 1;
    });

    BOOST_CHECK_EQUAL(b.value(), 2);
}

BOOST_AUTO_TEST_CASE(reevaluated_when_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed([=] {
        return a() + 1;
    });

    a = 5;

    BOOST_CHECK_EQUAL(b.value(), 6);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_1st_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return a() + b();
    });

    a = 5;

    BOOST_CHECK_EQUAL(c.value(), 7);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_2nd_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return a() + b();
    });

    b = 8;

    BOOST_CHECK_EQUAL(c.value(), 9);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_1st_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return a() + b();
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    a = 8;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_2nd_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return a() + b();
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    b = 8;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_on_each_change) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return a() + b();
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    b = 8;
    a = 10;
    b = 100;

    BOOST_CHECK_EQUAL(observer->notify_count, 3);
}

BOOST_AUTO_TEST_CASE(observers_not_called_after_removal) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return a() + b();
    });

    auto observer = std::make_shared<simple_observer>();

    {
        auto guard = with_observer(c, observer);

        b = 8;
    }

    a = 10;
    b = 100;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(all_observers_called) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return a() + b();
    });

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    auto guard1 = with_observer(c, observer1);

    b = 8;

    auto guard2 = with_observer(c, observer2);

    a = 10;
    b = 100;

    BOOST_CHECK_EQUAL(observer1->notify_count, 3);
    BOOST_CHECK_EQUAL(observer2->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(arguments_tracked_correctly_in_conditionals) {
    auto a = live_cells::variable(true);
    auto b = live_cells::variable(2);
    auto c = live_cells::variable(3);

    auto d = live_cells::computed([=] {
        return a() ? b() : c();
    });

    auto observer = std::make_shared<value_observer<int>>(d);
    auto guard = with_observer(d, observer);

    b = 1;
    a = false;
    c = 10;

    BOOST_CHECK(observer->values == std::vector<int>({1, 3, 10}));
}

BOOST_AUTO_TEST_CASE(dynamic_cell_argument_tracked_correctly) {
    auto a = live_cells::variable(true);
    auto b = live_cells::variable(2);
    auto c = live_cells::variable(3);

    auto d = live_cells::computed([=] {
        return a() ? b() : c();
    });

    auto e = live_cells::variable(0);
    auto f = live_cells::computed([=] {
        return d() + e();
    });

    auto observer = std::make_shared<value_observer<int>>(f);
    auto guard = with_observer(f, observer);

    b = 1;
    e = 10;
    a = false;
    c = 10;

    BOOST_CHECK(observer->values == std::vector<int>({1, 11, 13, 20}));
}

BOOST_AUTO_TEST_CASE(value_preserved_when_none_called) {
    auto a = live_cells::variable(10);
    auto evens = live_cells::computed([=] {
        if ((a() % 2) != 0) {
            live_cells::none();
        }

        return a();
    });

    auto observer = std::make_shared<value_observer<int>>(evens);
    auto guard = with_observer(evens, observer);

    a = 1;
    a = 2;
    a = 3;
    a = 4;
    a = 5;

    BOOST_CHECK(observer->values == std::vector<int>({10, 2, 4}));
}

BOOST_AUTO_TEST_CASE(exception_in_init_handled) {
    auto cell = live_cells::computed([=] {
        throw an_exception();
        return 0;
    });

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(exception_in_init_reproduced_on_access_while_observed) {
    auto cell = live_cells::computed([=] {
        throw an_exception();
        return 0;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(cell, observer);

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(compares_equal_if_same_key) {
    using live_cells::key_ref;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::cell c1 = live_cells::computed(key_ref::create<key_type>("the-key"), [=] {
        return a() + b();
    });

    live_cells::cell c2 = live_cells::computed(key_ref::create<key_type>("the-key"), [=] {
        return a() + b();
    });

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(c1 == c2);
    BOOST_CHECK(!(c1 != c2));
    BOOST_CHECK(hash(c1) == hash(c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_if_same_key) {
    using live_cells::key_ref;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::cell c1 = live_cells::computed(key_ref::create<key_type>("the-key1"), [=] {
        return a() + b();
    });

    live_cells::cell c2 = live_cells::computed(key_ref::create<key_type>("the-key2"), [=] {
        return a() + b();
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_with_default_key) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::cell c1 = live_cells::computed([=] {
        return a() + b();
    });

    live_cells::cell c2 = live_cells::computed([=] {
        return a() + b();
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_CASE(keyed_cells_manage_same_observers) {
    auto counter = std::make_shared<state_counter>();

    {
        auto key = live_cells::key_ref::create<value_key<std::string>>("key");

        auto a = test_managed_cell<int>(counter, 0);

        auto f = [=] {
            return live_cells::computed(key, [=] {
                return a() + 1;
            });
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

BOOST_AUTO_TEST_CASE(keyed_cell_state_reinitialized_on_add_observer_post_dispose) {
    auto counter = std::make_shared<state_counter>();
    auto key = live_cells::key_ref::create<value_key<std::string>>("key");

    auto a = test_managed_cell<int>(counter, 0);

    auto f = [=] {
        return live_cells::computed(key, [=] {
            return a() + 1;
        });
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
