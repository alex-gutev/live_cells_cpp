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
#include <sstream>

#include <boost/test/unit_test.hpp>

#include "live_cells.hpp"
#include "test_util.hpp"

BOOST_AUTO_TEST_SUITE(cell_state_consistency)

BOOST_AUTO_TEST_CASE(no_intermediate_value_with_multi_argument_cells) {
    auto a = live_cells::variable(0);

    auto sum = a + live_cells::value(1);
    auto prod = a * live_cells::value(8);
    auto result = sum + prod;

    auto observer = std::make_shared<value_observer<int>>(result);
    auto guard = with_observer(result, observer);

    a = 2;
    a = 6;

    BOOST_CHECK(observer->values == std::vector<int>({(2 + 1) + (2 * 8), (6 + 1) + (6 * 8)}));
}

BOOST_AUTO_TEST_CASE(no_intermediate_value_with_store_cells) {
    auto a = live_cells::variable(0);

    auto sum = a + live_cells::value(1) | live_cells::ops::store;
    auto prod = a * live_cells::value(8) | live_cells::ops::store;
    auto result = sum + prod;

    auto observer = std::make_shared<value_observer<int>>(result);
    auto guard = with_observer(result, observer);

    a = 2;
    a = 6;

    BOOST_CHECK(observer->values == std::vector<int>({(2 + 1) + (2 * 8), (6 + 1) + (6 * 8)}));
}

BOOST_AUTO_TEST_CASE(no_intermediate_value_with_store_cells_and_unequal_branches) {
    using live_cells::store;
    using live_cells::value;

    auto a = live_cells::variable(0);

    auto sum = store(store(a + value(1)) + value(10));
    auto prod = store(a * value(8));
    auto result = store(sum + prod);

    auto observer = std::make_shared<value_observer<int>>(result);
    auto guard = with_observer(result, observer);

    a = 2;
    a = 6;

    BOOST_CHECK(observer->values == std::vector<int>({
                (2 + 1 + 10) + (2 * 8),
                (6 + 1 + 10) + (6 * 8)
            }));
}

BOOST_AUTO_TEST_CASE(no_intermediate_value_with_dynamic_compute_cell) {
    auto a = live_cells::variable(0);

    auto sum = live_cells::computed([=] {
        return a() + 1;
    });

    auto prod = live_cells::computed([=] {
        return a() * 8;
    });

    auto result = live_cells::computed([=] {
        return sum() + prod();
    });

    auto observer = std::make_shared<value_observer<int>>(result);
    auto guard = with_observer(result, observer);

    a = 2;
    a = 6;

    BOOST_CHECK(observer->values == std::vector<int>({(2 + 1) + (2 * 8), (6 + 1) + (6 * 8)}));
}

BOOST_AUTO_TEST_CASE(no_intermediate_value_with_dynamic_compute_cells_and_unequal_branches) {
    auto a = live_cells::variable(0);

    auto sum1 = live_cells::computed([=] {
        return a() + 1;
    });
    auto sum = live_cells::computed([=] {
        return sum1() + 10;
    });

    auto prod = live_cells::computed([=] {
        return a() * 8;
    });

    auto result = live_cells::computed([=] {
        return sum() + prod();
    });

    auto observer = std::make_shared<value_observer<int>>(result);
    auto guard = with_observer(result, observer);

    a = 2;
    a = 6;

    BOOST_CHECK(observer->values == std::vector<int>({
                (2 + 1 + 10) + (2 * 8),
                (6 + 1 + 10) + (6 * 8)
            }));
}

BOOST_AUTO_TEST_CASE(no_intermediate_values_with_batch_update) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(0);
    auto op = live_cells::variable<std::string>("");

    auto sum = a + b;
    auto msg = live_cells::computed(a, b, op, sum, [] (auto a, auto b, auto op, auto sum) {
        std::stringstream ss;

        ss << a;
        ss << " ";
        ss << op;
        ss << " ";
        ss << b;
        ss << " = ";
        ss << sum;

        return ss.str();
    });

    auto observer = std::make_shared<value_observer<std::string>>(msg);
    auto guard = with_observer(msg, observer);

    live_cells::batch([&] {
        a = 1;
        b = 2;
        op = "+";
    });

    {
        live_cells::batch_update guard;

        a = 5;
        b = 6;
        op = "plus";
    }

    BOOST_CHECK(observer->values == std::vector<std::string>({
                "1 + 2 = 3",
                "5 plus 6 = 11"
            }));
}

BOOST_AUTO_TEST_CASE(no_intermediate_values_with_batch_and_store_cells) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(0);
    auto op = live_cells::variable<std::string>("");

    auto sum = a + b | live_cells::ops::store;
    auto msg = live_cells::computed(a, b, op, sum, [] (auto a, auto b, auto op, auto sum) {
        std::stringstream ss;

        ss << a;
        ss << " ";
        ss << op;
        ss << " ";
        ss << b;
        ss << " = ";
        ss << sum;

        return ss.str();
    });

    auto observer = std::make_shared<value_observer<std::string>>(msg);
    auto guard = with_observer(msg, observer);

    live_cells::batch([&] {
        a = 1;
        b = 2;
        op = "+";
    });

    {
        live_cells::batch_update guard;

        a = 5;
        b = 6;
        op = "plus";
    }

    BOOST_CHECK(observer->values == std::vector<std::string>({
                "1 + 2 = 3",
                "5 plus 6 = 11"
            }));
}

BOOST_AUTO_TEST_CASE(no_intermediate_values_with_batch_and_dynamic_compute_cell) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);
    auto c = live_cells::variable(3);
    auto select = live_cells::variable(true);

    auto sum = live_cells::computed([=] {
        return a() + b();
    });

    auto result = live_cells::computed([=] {
        return select() ? c() : sum();
    });

    auto observer = std::make_shared<value_observer<int>>(result);
    auto guard = with_observer(result, observer);

    live_cells::batch([&] {
        select = true;
        c = 10;
        a = 5;
    });

    {
        live_cells::batch_update guard;

        b = 20;
        select = false;
    }

    BOOST_CHECK(observer->values == std::vector<int>({10, 25}));
}

BOOST_AUTO_TEST_CASE(all_store_cell_observers_called_correct_number_of_times) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);
    auto sum = a + b | live_cells::ops::store;

    auto c = a + sum | live_cells::ops::store;
    auto d = sum + live_cells::value(2);

    auto observer_c = std::make_shared<simple_observer>();
    auto observer_d = std::make_shared<simple_observer>();

    auto guard1 = with_observer(c, observer_c);
    auto guard2 = with_observer(d, observer_d);

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

    BOOST_CHECK_EQUAL(observer_c->notify_count, 3);
    BOOST_CHECK_EQUAL(observer_d->notify_count, 3);
}

BOOST_AUTO_TEST_CASE(correct_values_produced_across_all_store_cell_observers) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);
    auto sum = a + b | live_cells::ops::store;

    auto c = a + sum | live_cells::ops::store;
    auto d = sum + live_cells::value(2);

    auto observer_c = std::make_shared<value_observer<int>>(c);
    auto observer_d = std::make_shared<value_observer<int>>(d);

    auto guard1 = with_observer(c, observer_c);
    auto guard2 = with_observer(d, observer_d);

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

    BOOST_CHECK(observer_c->values == std::vector<int>({7, 8, 40}));
    BOOST_CHECK(observer_d->values == std::vector<int>({7, 32}));
}

BOOST_AUTO_TEST_SUITE_END();
