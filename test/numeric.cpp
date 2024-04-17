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
#include "numeric.hpp"

#include "test_util.hpp"


BOOST_AUTO_TEST_SUITE(numeric_operators)

BOOST_AUTO_TEST_CASE(additon) {
    auto a = live_cells::value(5);
    auto b = live_cells::value(6);

    auto c1 = a + b;
    auto c2 = 5 + b;
    auto c3 = a + 6;

    BOOST_CHECK_EQUAL(c1.value(), 11);
    BOOST_CHECK_EQUAL(c2.value(), 11);
    BOOST_CHECK_EQUAL(c3.value(), 11);
}

BOOST_AUTO_TEST_CASE(subtraction) {
    auto a = live_cells::value(13);
    auto b = live_cells::value(20);

    auto c1 = a - b;
    auto c2 = 13 - b;
    auto c3 = a - 20;

    BOOST_CHECK_EQUAL(c1.value(), -7);
    BOOST_CHECK_EQUAL(c2.value(), -7);
    BOOST_CHECK_EQUAL(c3.value(), -7);
}

BOOST_AUTO_TEST_CASE(multiplication) {
    auto a = live_cells::value(10);
    auto b = live_cells::value(8);

    auto c1 = a * b;
    auto c2 = 10 * b;
    auto c3 = a * 8;

    BOOST_CHECK_EQUAL(c1.value(), 80);
    BOOST_CHECK_EQUAL(c2.value(), 80);
    BOOST_CHECK_EQUAL(c3.value(), 80);
}

BOOST_AUTO_TEST_CASE(division) {
    auto a = live_cells::value(8);
    auto b = live_cells::value(4);

    auto c1 = a / b;
    auto c2 = 8 / b;
    auto c3 = a / 4;

    BOOST_CHECK_EQUAL(c1.value(), 2);
    BOOST_CHECK_EQUAL(c2.value(), 2);
    BOOST_CHECK_EQUAL(c3.value(), 2);
}

BOOST_AUTO_TEST_CASE(modulu) {
    auto a = live_cells::value(17);
    auto b = live_cells::value(3);

    auto c1 = a % b;
    auto c2 = a % 3;
    auto c3 = 17 % b;

    BOOST_CHECK_EQUAL(c1.value(), 2);
    BOOST_CHECK_EQUAL(c2.value(), 2);
    BOOST_CHECK_EQUAL(c3.value(), 2);
}

BOOST_AUTO_TEST_CASE(less_than) {
    auto a = live_cells::value(3);
    auto b = live_cells::value(8);

    auto lt = a < b;
    auto gt = b < a;
    auto eq = a < a;

    auto c1 = a < 8;
    auto c2 = 3 < b;

    BOOST_CHECK(lt.value());
    BOOST_CHECK(!gt.value());
    BOOST_CHECK(!eq.value());

    BOOST_CHECK(c1.value());
    BOOST_CHECK(c2.value());
}

BOOST_AUTO_TEST_CASE(less_than_or_equal) {
    auto a = live_cells::value(3);
    auto b = live_cells::value(8);

    auto lt = a <= b;
    auto gt = b <= a;
    auto eq = a <= a;

    auto c1 = a <= 8;
    auto c2 = 3 <= b;

    BOOST_CHECK(lt.value());
    BOOST_CHECK(!gt.value());
    BOOST_CHECK(eq.value());

    BOOST_CHECK(c1.value());
    BOOST_CHECK(c2.value());
}

BOOST_AUTO_TEST_CASE(greater_than) {
    auto a = live_cells::value(3);
    auto b = live_cells::value(8);

    auto gt = a > b;
    auto lt = b > a;
    auto eq = a > a;

    auto c1 = a > 8;
    auto c2 = 3 > b;

    BOOST_CHECK(!gt.value());
    BOOST_CHECK(lt.value());
    BOOST_CHECK(!eq.value());

    BOOST_CHECK(!c1.value());
    BOOST_CHECK(!c2.value());
}

BOOST_AUTO_TEST_CASE(greater_than_or_equal) {
    auto a = live_cells::value(3);
    auto b = live_cells::value(8);

    auto gt = a >= b;
    auto lt = b >= a;
    auto eq = a >= a;

    auto c1 = a >= 8;
    auto c2 = 3 >= b;

    BOOST_CHECK(!gt.value());
    BOOST_CHECK(lt.value());
    BOOST_CHECK(eq.value());

    BOOST_CHECK(!c1.value());
    BOOST_CHECK(!c2.value());
}

BOOST_AUTO_TEST_SUITE_END()
