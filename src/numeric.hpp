/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
