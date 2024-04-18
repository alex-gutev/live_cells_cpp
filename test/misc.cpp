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

BOOST_AUTO_TEST_SUITE_END()
