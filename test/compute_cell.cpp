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

    a.value(5);

    BOOST_CHECK_EQUAL(b.value(), 6);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_1st_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    a.value(5);

    BOOST_CHECK_EQUAL(c.value(), 7);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_2nd_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    b.value(8);

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

    a.value(8);

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

    b.value(8);

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

    b.value(8);
    a.value(10);
    b.value(100);

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

        b.value(8);
    }

    a.value(10);
    b.value(100);

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

    b.value(8);

    auto guard2 = with_observer(c, observer2);

    a.value(10);
    b.value(100);

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
