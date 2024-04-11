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

BOOST_AUTO_TEST_SUITE(compute_cell)

BOOST_AUTO_TEST_CASE(compute_cell_applied_on_constant_cell_value) {
    auto a = live_cells::value(1);
    auto b = live_cells::computed(a, [] (auto a) {
        return a + 1;
    });

    BOOST_CHECK_EQUAL(b.value(), 2);
}

BOOST_AUTO_TEST_CASE(compute_cell_reevaluated_when_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed(a, [] (auto a) {
        return a + 1;
    });

    a = 5;

    BOOST_CHECK_EQUAL(b.value(), 6);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_1st_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    a = 5;

    BOOST_CHECK_EQUAL(c.value(), 7);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_2nd_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    b = 8;

    BOOST_CHECK_EQUAL(c.value(), 9);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_1st_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    a = 8;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_2nd_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    b = 8;

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_on_each_change) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
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

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
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

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
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

BOOST_AUTO_TEST_CASE(compares_equal_if_same_key) {
    using live_cells::key_ref;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::cell c1 = live_cells::computed(key_ref::create<key_type>("the-key"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    live_cells::cell c2 = live_cells::computed(key_ref::create<key_type>("the-key"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(c1 == c2);
    BOOST_CHECK(!(c1 != c2));
    BOOST_CHECK(hash(c1) == hash(c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_if_different_key) {
    using live_cells::key_ref;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::cell c1 = live_cells::computed(key_ref::create<key_type>("the-key1"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    live_cells::cell c2 = live_cells::computed(key_ref::create<key_type>("the-key2"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_with_default_key) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::cell c1 = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    live_cells::cell c2 = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_SUITE_END()
