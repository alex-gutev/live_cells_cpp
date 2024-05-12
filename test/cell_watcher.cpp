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
#include "computed.hpp"
#include "numeric.hpp"
#include "store_cell.hpp"
#include "mutable_computed.hpp"

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

BOOST_AUTO_TEST_SUITE(cell_watcher_changes_only)

BOOST_AUTO_TEST_CASE(not_called_when_value_unchanged) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(b());
    });

    a = {4, 2, 6};
    a = {7, 8, 9};

    BOOST_CHECK(values == std::vector({2, 8}));
}

BOOST_AUTO_TEST_CASE(not_called_when_value_unchanged_in_batch) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(b());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
    });

    live_cells::batch([&] {
        a = {7, 8, 9};
    });

    BOOST_CHECK(values == std::vector({2, 8}));
}

BOOST_AUTO_TEST_CASE(called_when_one_argument_changes) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::variable(3);

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(b());
        values.push_back(c());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
        c = 5;
    });

    BOOST_CHECK(values == std::vector({2, 3, 2, 5}));
}

BOOST_AUTO_TEST_CASE(computed_cell_not_recomputed_when_arguments_not_changed) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::computed([=] {
        return b() * 10;
    });

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(c());
    });

    a = {4, 2, 6};
    a = {7, 8, 9};

    BOOST_CHECK(values == std::vector({20, 80}));
}

BOOST_AUTO_TEST_CASE(computed_cell_not_recomputed_when_arguments_not_changed_in_batch) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::computed([=] {
        return b() * 10;
    });

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(c());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
    });

    live_cells::batch([&] {
        a = {7, 8, 9};
    });

    BOOST_CHECK(values == std::vector({20, 80}));
}

BOOST_AUTO_TEST_CASE(computed_cell_recomputed_when_one_argument_changes) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::variable(3);
    auto d = live_cells::computed([=] {
        return b() * c();
    });

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(d());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
        c = 5;
    });

    BOOST_CHECK(values == std::vector({6, 10}));
}

BOOST_AUTO_TEST_CASE(store_cell_not_recomputed_when_argument_value_unchanged) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = (b * 10) | live_cells::ops::store;

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(c());
    });

    a = {4, 2, 6};
    a = {7, 8, 9};

    BOOST_CHECK(values == std::vector({20, 80}));
}

BOOST_AUTO_TEST_CASE(store_cell_not_recomputed_when_argument_value_unchanged_in_batch) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = (b * 10) | live_cells::ops::store;

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(c());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
    });

    live_cells::batch([&] {
        a = {7, 8, 9};
    });

    BOOST_CHECK(values == std::vector({20, 80}));
}

BOOST_AUTO_TEST_CASE(store_cell_recomputed_when_one_argument_changed) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::variable(3);
    auto d = (b * c) | live_cells::ops::store;

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(d());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
        c = 5;
    });

    BOOST_CHECK(values == std::vector({6, 10}));
}

BOOST_AUTO_TEST_CASE(store_cell_not_recomputed_when_value_unchanged) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(a, [] (auto a) {
        return a[1];
    }) | live_cells::ops::cache;

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(b());
    });

    a = {4, 2, 6};
    a = {7, 2, 8};
    a = {9, 10, 11};

    BOOST_CHECK(values == std::vector({2, 10}));
}

BOOST_AUTO_TEST_CASE(static_mutable_compute_cell_not_recomputed_when_arguments_unchanged) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::mutable_computed(b, [] (auto b) {
        return b + 10;
    }, [] (auto) {});

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(c());
    });

    a = {4, 2, 6};
    a = {7, 8, 9};

    BOOST_CHECK(values == std::vector({12, 18}));
}

BOOST_AUTO_TEST_CASE(static_mutable_compute_cell_not_recomputed_when_arguments_unchanged_in_batch) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::mutable_computed(b, [] (auto b) {
        return b + 10;
    }, [] (auto) {});

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(c());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
    });

    live_cells::batch([&] {
        a = {7, 8, 9};
    });

    BOOST_CHECK(values == std::vector({12, 18}));
}

BOOST_AUTO_TEST_CASE(static_mutable_compute_cell_recomputed_when_one_argument_changed) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::computed(live_cells::changes_only(), [=] {
        return a()[1];
    });

    auto c = live_cells::variable(3);

    auto d = live_cells::mutable_computed(b, c, [] (auto b, auto c) {
        return b * c;
    }, [] (auto) {});

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(d());
    });

    live_cells::batch([&] {
        a = {4, 2, 6};
        c = 5;
    });

    live_cells::batch([&] {
        a = {7, 8, 9};
    });

    BOOST_CHECK(values == std::vector({6, 10, 40}));
}

BOOST_AUTO_TEST_CASE(static_mutable_compute_cell_not_recomputed_when_value_unchanged) {
    auto a = live_cells::variable(std::vector({1, 2, 3}));
    auto b = live_cells::mutable_computed(live_cells::changes_only(), a, [] (auto a) {
        return a[1];
    }, [] (auto) {});

    std::vector<int> values;

    auto watch = live_cells::watch([&] {
        values.push_back(b());
    });

    a = {4, 2, 6};
    a = {7, 2, 8};
    a = {9, 10, 11};

    BOOST_CHECK(values == std::vector({2, 10}));
}

BOOST_AUTO_TEST_SUITE_END();
