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
#include "mutable_computed.hpp"
#include "numeric.hpp"
#include "store_cell.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(mutable_compute_cell)

BOOST_AUTO_TEST_CASE(value_computed_on_construction) {
    auto a = live_cells::variable(1);

    auto b = live_cells::mutable_computed(a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    BOOST_CHECK_EQUAL(b.value(), 2);
}

BOOST_AUTO_TEST_CASE(value_recomputed_when_argument_cell_changes) {
    auto a = live_cells::variable(1);

    auto b = live_cells::mutable_computed(a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    auto guard = with_observer(b, std::make_shared<simple_observer>());

    a = 5;
    BOOST_CHECK_EQUAL(b.value(), 6);
}

BOOST_AUTO_TEST_CASE(value_recomputed_when_1st_argument_cell_changes) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto guard = with_observer(c, std::make_shared<simple_observer>());

    a = 5;
    BOOST_CHECK_EQUAL(c.value(), 8);
}

BOOST_AUTO_TEST_CASE(value_recomputed_when_2nd_argument_cell_changes) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto guard = with_observer(c, std::make_shared<simple_observer>());

    b = 9;
    BOOST_CHECK_EQUAL(c.value(), 10);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_value_recomputed) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    b = 9;
    a = 10;

    BOOST_CHECK_EQUAL(observer->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(observer_not_called_after_removal) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    auto guard1 = with_observer(c, observer1);

    {
        auto guard2 = with_observer(c, observer2);
        b = 9;
    }

    a = 10;

    BOOST_CHECK_EQUAL(observer1->notify_count, 2);
    BOOST_CHECK_EQUAL(observer2->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(set_value_updates_argument_cell_values) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    c = 10;

    BOOST_CHECK_EQUAL(a.value(), 5);
    BOOST_CHECK_EQUAL(b.value(), 5);
    BOOST_CHECK_EQUAL(c.value(), 10);
}

BOOST_AUTO_TEST_CASE(set_value_notifies_observers) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto obs_a = std::make_shared<simple_observer>();
    auto obs_b = std::make_shared<simple_observer>();
    auto obs_c = std::make_shared<simple_observer>();

    auto guard1 = with_observer(a, obs_a);
    auto guard2 = with_observer(b, obs_b);
    auto guard3 = with_observer(c, obs_c);

    c = 10;

    BOOST_CHECK_EQUAL(obs_a->notify_count, 1);
    BOOST_CHECK_EQUAL(obs_b->notify_count, 1);
    BOOST_CHECK_EQUAL(obs_c->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(every_set_value_notifies_observers) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto obs_a = std::make_shared<simple_observer>();
    auto obs_b = std::make_shared<simple_observer>();
    auto obs_c = std::make_shared<simple_observer>();

    auto guard1 = with_observer(a, obs_a);
    auto guard2 = with_observer(b, obs_b);
    auto guard3 = with_observer(c, obs_c);

    c = 10;
    c = 12;

    BOOST_CHECK_EQUAL(obs_a->notify_count, 2);
    BOOST_CHECK_EQUAL(obs_b->notify_count, 2);
    BOOST_CHECK_EQUAL(obs_c->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(consistent_state_when_setting_value_in_batch) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto d = live_cells::variable(50);
    auto e = c + d | live_cells::ops::store;

    auto obs_a = std::make_shared<value_observer<double>>(a);
    auto obs_b = std::make_shared<value_observer<double>>(b);
    auto obs_c = std::make_shared<value_observer<double>>(c);
    auto obs_e = std::make_shared<value_observer<double>>(e);

    auto guard1 = with_observer(a, obs_a);
    auto guard2 = with_observer(b, obs_b);
    auto guard3 = with_observer(c, obs_c);
    auto guard5 = with_observer(e, obs_e);

    live_cells::batch([&] {
        c = 10;
        d = 9;
    });

    obs_a->check_values({5});
    obs_b->check_values({5});
    obs_c->check_values({10});
    obs_e->check_values({19});
}

BOOST_AUTO_TEST_CASE(observers_notified_correct_number_of_times_when_set_value_in_batch) {
    auto a = live_cells::variable(1.0);
    auto b = live_cells::variable(3.0);

    auto c = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto d = live_cells::variable(50);
    auto e = c + d | live_cells::ops::store;

    auto obs_a = std::make_shared<simple_observer>();
    auto obs_b = std::make_shared<simple_observer>();
    auto obs_c = std::make_shared<simple_observer>();
    auto obs_e = std::make_shared<simple_observer>();

    auto guard1 = with_observer(a, obs_a);
    auto guard2 = with_observer(b, obs_b);
    auto guard3 = with_observer(c, obs_c);
    auto guard5 = with_observer(e, obs_e);

    live_cells::batch([&] {
        c = 10;
        d = 9;
    });

    BOOST_CHECK_EQUAL(obs_a->notify_count, 1);
    BOOST_CHECK_EQUAL(obs_b->notify_count, 1);
    BOOST_CHECK_EQUAL(obs_c->notify_count, 1);
    BOOST_CHECK_EQUAL(obs_e->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(all_observers_notified_correct_number_of_times_when_set_value_in_batch) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto sum = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto c = a + sum | live_cells::ops::store;
    auto d = sum + live_cells::value(2);

    auto obs_c = std::make_shared<simple_observer>();
    auto obs_d = std::make_shared<simple_observer>();

    auto guard1 = with_observer(c, obs_c);
    auto guard2 = with_observer(d, obs_d);

    live_cells::batch([&] {
        a = 2;
        b = 3;
    });

    live_cells::batch([&] {
        a = 3;
        b = 2;
    });

    live_cells::batch([&] {
        a = 10;
        b = 20;
    });

    BOOST_CHECK_EQUAL(obs_c->notify_count, 3);
    BOOST_CHECK_EQUAL(obs_d->notify_count, 3);
}

BOOST_AUTO_TEST_CASE(correct_values_produced_across_all_observers) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto sum = live_cells::mutable_computed(a, b, [] (auto a, auto b) {
        return a + b;
    }, [=] (auto c) {
        auto half = c / 2;

        a = half;
        b = half;
    });

    auto c = a + sum | live_cells::ops::store;
    auto d = sum + live_cells::value(2);

    auto obs_c = std::make_shared<value_observer<int>>(c);
    auto obs_d = std::make_shared<value_observer<int>>(d);

    auto guard1 = with_observer(c, obs_c);
    auto guard2 = with_observer(d, obs_d);

    live_cells::batch([&] {
        a = 2;
        b = 3;
    });

    live_cells::batch([&] {
        a = 3;
        b = 2;
    });

    live_cells::batch([&] {
        a = 10;
        b = 20;
    });

    obs_c->check_values({7, 8, 40});
    obs_d->check_values({7, 32});
}

BOOST_AUTO_TEST_CASE(previous_value_preserved_when_none_used) {
    auto a = live_cells::variable(0);

    auto evens = live_cells::mutable_computed(a, [] (auto a) {
        if (a % 2) {
            live_cells::none();
        }

        return a;
    }, [=] (auto v) {
        a = v;
    });

    auto obs = std::make_shared<value_observer<int>>(evens);
    auto guard = with_observer(evens, obs);

    a = 1;
    a = 2;
    a = 3;
    a = 4;
    a = 5;

    obs->check_values({0, 2, 4});
}

BOOST_AUTO_TEST_CASE(exception_in_init_handled) {
    auto a = live_cells::variable(0);
    auto cell = live_cells::mutable_computed(a, [] (auto a) {
        if (a == 0) {
            throw an_exception();
        }

        return a;
    }, [=] (auto v) {
        a = v;
    });

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(exception_in_init_reproduced_on_access_while_observed) {
    auto a = live_cells::variable(0);
    auto cell = live_cells::mutable_computed(a, [] (auto a) {
        if (a == 0) {
            throw an_exception();
        }

        return a;
    }, [=] (auto v) {
        a = v;
    });

    auto guard = with_observer(cell, std::make_shared<simple_observer>());

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(chained_mutable_compute_cells) {
    auto a = live_cells::variable(0);

    auto b = live_cells::mutable_computed(a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    auto c = live_cells::mutable_computed(b, [] (auto b) {
        return b + 1;
    }, [=] (auto c) {
        b = c - 1;
    });

    auto obs_a = std::make_shared<value_observer<int>>(a);
    auto obs_b = std::make_shared<value_observer<int>>(b);

    auto guard1 = with_observer(a, obs_a);
    auto guard2 = with_observer(b, obs_b);

    b = 3;
    c = 10;

    obs_a->check_values({2, 8});
    obs_b->check_values({3, 9});
}

BOOST_AUTO_TEST_CASE(compares_equal_if_same_key) {
    using live_cells::key_ref;
    using live_cells::mutable_computed;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);

    live_cells::cell c1 = mutable_computed(key_ref::create<key_type>("the-key"), a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    live_cells::cell c2 = mutable_computed(key_ref::create<key_type>("the-key"), a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    std::hash<live_cells::cell> hash;

    BOOST_CHECK(c1 == c2);
    BOOST_CHECK(!(c1 != c2));
    BOOST_CHECK(hash(c1) == hash(c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_if_different_key) {
    using live_cells::key_ref;
    using live_cells::mutable_computed;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);

    live_cells::cell c1 = mutable_computed(key_ref::create<key_type>("the-key1"), a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    live_cells::cell c2 = mutable_computed(key_ref::create<key_type>("the-key2"), a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_with_default_key) {
    using live_cells::key_ref;
    using live_cells::mutable_computed;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);

    live_cells::cell c1 = mutable_computed(a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    live_cells::cell c2 = mutable_computed(a, [] (auto a) {
        return a + 1;
    }, [=] (auto b) {
        a = b - 1;
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_SUITE_END();
