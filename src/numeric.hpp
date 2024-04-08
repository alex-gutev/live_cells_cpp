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

#ifndef LIVE_CELLS_NUMERIC_HPP
#define LIVE_CELLS_NUMERIC_HPP

#include "observable.hpp"
#include "computed.hpp"

namespace live_cells {
    /**
     * Returns a computed cell that evaluates `a + b` on the values
     * held in cells `a` and `b`.
     */
    auto operator +(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a + b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a - b` on the values
     * held in cells `a` and `b`.
     */
    auto operator -(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a - b;
        });
    }

    /**
     * Returns a computed cell that evaluates to `-cell`, on the value
     * held in `cell`.
     */
    auto operator -(const Cell auto &cell) {
        return computed(cell, [] (auto value) {
            return -value;
        });
    }

    /**
     * Returns a computed cell that evaluates `a * b` on the values
     * held in cells `a` and `b`.
     */
    auto operator *(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a * b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a / b` on the values
     * held in cells `a` and `b`.
     */
    auto operator /(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a / b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a % b` on the values
     * held in cells `a` and `b`.
     */
    auto operator %(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a % b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a < b` on the values
     * held in cells `a` and `b`.
     */
    auto operator <(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a < b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a <= b` on the values
     * held in cells `a` and `b`.
     */
    auto operator <=(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a <= b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a > b` on the values
     * held in cells `a` and `b`.
     */
    auto operator >(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a > b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a >= b` on the values
     * held in cells `a` and `b`.
     */
    auto operator >=(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a >= b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_NUMERIC_HPP */
