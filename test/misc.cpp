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

#include <boost/test/unit_test.hpp>

#include "live_cells.hpp"

BOOST_AUTO_TEST_SUITE(miscellaneous)

BOOST_AUTO_TEST_CASE(pipeline) {
    auto cond = live_cells::variable(true);
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);
    auto c = live_cells::variable(2);

    auto cell = cond
        | live_cells::ops::select(a, b)
        | live_cells::ops::on_error(c)
        | live_cells::ops::peek;

    BOOST_CHECK_EQUAL(cell.value(), 0);

    cond = false;
    BOOST_CHECK_EQUAL(cell.value(), 1);
}

BOOST_AUTO_TEST_CASE(plus_assignment) {
    auto a = live_cells::variable(1);

    BOOST_CHECK_EQUAL(a.value(), 1);

    BOOST_CHECK_EQUAL(a += 1, 2);
    BOOST_CHECK_EQUAL(a.value(), 2);

    a += 10;
    BOOST_CHECK_EQUAL(a.value(), 12);
}

BOOST_AUTO_TEST_CASE(minus_assignment) {
    auto a = live_cells::variable(2);

    BOOST_CHECK_EQUAL(a.value(), 2);

    a -= 1;
    BOOST_CHECK_EQUAL(a.value(), 1);

    BOOST_CHECK_EQUAL(a -= 10, -9);
    BOOST_CHECK_EQUAL(a.value(), -9);
}

BOOST_AUTO_TEST_CASE(times_assignment) {
    auto a = live_cells::variable(2);

    BOOST_CHECK_EQUAL(a.value(), 2);

    BOOST_CHECK_EQUAL(a *= 2, 4);
    BOOST_CHECK_EQUAL(a.value(), 4);

    a *= 10;
    BOOST_CHECK_EQUAL(a.value(), 40);
}

BOOST_AUTO_TEST_CASE(divide_assignment) {
    auto a = live_cells::variable(40);

    BOOST_CHECK_EQUAL(a.value(), 40);

    BOOST_CHECK_EQUAL(a /= 2, 20);
    BOOST_CHECK_EQUAL(a.value(), 20);

    a /= 10;
    BOOST_CHECK_EQUAL(a.value(), 2);
}

BOOST_AUTO_TEST_CASE(remainder_assignment) {
    auto a = live_cells::variable(7);

    BOOST_CHECK_EQUAL(a.value(), 7);

    BOOST_CHECK_EQUAL(a %= 4, 3);
    BOOST_CHECK_EQUAL(a.value(), 3);

    a %= 2;
    BOOST_CHECK_EQUAL(a.value(), 1);
}

BOOST_AUTO_TEST_CASE(left_shift_assignment) {
    auto a = live_cells::variable(2);

    BOOST_CHECK_EQUAL(a.value(), 2);

    BOOST_CHECK_EQUAL(a <<= 1, 4);
    BOOST_CHECK_EQUAL(a.value(), 4);

    a <<= 2;
    BOOST_CHECK_EQUAL(a.value(), 16);
}

BOOST_AUTO_TEST_CASE(right_shift_assignment) {
    auto a = live_cells::variable(16);

    BOOST_CHECK_EQUAL(a.value(), 16);

    BOOST_CHECK_EQUAL(a >>= 1, 8);
    BOOST_CHECK_EQUAL(a.value(), 8);

    a >>= 2;
    BOOST_CHECK_EQUAL(a.value(), 2);
}

BOOST_AUTO_TEST_CASE(and_assignment) {
    auto a = live_cells::variable(0xFF);

    BOOST_CHECK_EQUAL(a.value(), 0xFF);

    a &= 0x05;
    BOOST_CHECK_EQUAL(a.value(), 0x05);

    BOOST_CHECK_EQUAL(a &= 0x06, 0x04);
    BOOST_CHECK_EQUAL(a.value(), 0x04);
}

BOOST_AUTO_TEST_CASE(or_assignment) {
    auto a = live_cells::variable(0);

    BOOST_CHECK_EQUAL(a.value(), 0);

    a |= 0x05;
    BOOST_CHECK_EQUAL(a.value(), 0x05);

    BOOST_CHECK_EQUAL(a |= 0x06, 0x07);
    BOOST_CHECK_EQUAL(a.value(), 0x07);
}

BOOST_AUTO_TEST_CASE(xor_assignment) {
    auto a = live_cells::variable(0);

    BOOST_CHECK_EQUAL(a.value(), 0);

    a ^= 0x05;
    BOOST_CHECK_EQUAL(a.value(), 0x05);

    BOOST_CHECK_EQUAL(a ^= 0x06, 0x03);
    BOOST_CHECK_EQUAL(a.value(), 0x03);
}

BOOST_AUTO_TEST_CASE(plusplus_prefix) {
    auto a = live_cells::variable(0);

    BOOST_CHECK_EQUAL(a.value(), 0);

    BOOST_CHECK_EQUAL(++a, 1);
    BOOST_CHECK_EQUAL(a.value(), 1);

    BOOST_CHECK_EQUAL(++a, 2);
    BOOST_CHECK_EQUAL(a.value(), 2);
}

BOOST_AUTO_TEST_CASE(plusplus_postfix) {
    auto a = live_cells::variable(0);

    BOOST_CHECK_EQUAL(a.value(), 0);

    BOOST_CHECK_EQUAL(a++, 0);
    BOOST_CHECK_EQUAL(a.value(), 1);

    BOOST_CHECK_EQUAL(a++, 1);
    BOOST_CHECK_EQUAL(a.value(), 2);
}

BOOST_AUTO_TEST_CASE(minusminus_prefix) {
    auto a = live_cells::variable(1);

    BOOST_CHECK_EQUAL(a.value(), 1);

    BOOST_CHECK_EQUAL(--a, 0);
    BOOST_CHECK_EQUAL(a.value(), 0);

    BOOST_CHECK_EQUAL(--a, -1);
    BOOST_CHECK_EQUAL(a.value(), -1);
}

BOOST_AUTO_TEST_CASE(minusminus_postfix) {
    auto a = live_cells::variable(1);

    BOOST_CHECK_EQUAL(a.value(), 1);

    BOOST_CHECK_EQUAL(a--, 1);
    BOOST_CHECK_EQUAL(a.value(), 0);

    BOOST_CHECK_EQUAL(a--, 0);
    BOOST_CHECK_EQUAL(a.value(), -1);
}

BOOST_AUTO_TEST_CASE(cast_cell_to_typed_cell) {
    live_cells::cell a = live_cells::variable(1);
    live_cells::typed_cell<int> b = a;

    BOOST_CHECK_EQUAL(a.value<int>(), 1);
    BOOST_CHECK_EQUAL(b.value(), 1);

    a.value<int>(4);
    BOOST_CHECK_EQUAL(a.value<int>(), 4);
    BOOST_CHECK_EQUAL(b.value(), 4);
}

BOOST_AUTO_TEST_CASE(bad_cast_cell_to_typed_cell) {
    live_cells::cell a = live_cells::variable(1);

    BOOST_CHECK_THROW((live_cells::typed_cell<std::string>(a)), live_cells::bad_typed_cell_cast);
}

BOOST_AUTO_TEST_CASE(cast_typed_cell_to_cell) {
    live_cells::typed_cell<int> a = live_cells::variable(1);
    live_cells::cell b = a;

    BOOST_CHECK_EQUAL(a.value(), 1);
    BOOST_CHECK_EQUAL(b.value<int>(), 1);

    a.value(4);
    BOOST_CHECK_EQUAL(a.value(), 4);
    BOOST_CHECK_EQUAL(b.value<int>(), 4);
}

BOOST_AUTO_TEST_SUITE_END()
