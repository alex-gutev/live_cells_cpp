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

#ifndef LIVE_CELLS_BOOLEAN_HPP
#define LIVE_CELLS_BOOLEAN_HPP

#include "observable.hpp"
#include "computed.hpp"
#include "store_cell.hpp"

namespace live_cells {
    /**
     * Returns a cell which evaluates to the logical and of the values
     * of cells `a` and `b`.
     *
     * NOTE: This operator is short-circuiting which means that the
     * value of cell `b` is only referenced if `a` is true.
     */
    auto operator && (const Cell auto &a, const Cell auto &b) {
        return make_compute_cell([=] () {
            return a.value() && b.value();
        }, a, b);
    }

    /**
     * Returns a cell which evaluates to the logical or of the values
     * of cells `a` and `b`.
     *
     * NOTE: This operator is short-circuiting which means that the
     * value of cell `b` is only referenced if `a` is false.
     */
    auto operator || (const Cell auto &a, const Cell auto &b) {
        return make_compute_cell([=] () {
            return a.value() || b.value();
        }, a, b);
    }

    /**
     * Returns a cell which evaluates to the logical not of the value
     * of a cell.
     */
    auto operator ! (const Cell auto &cell) {
        return computed(cell, [] (auto value) {
            return !value;
        });
    }

    /**
     * \brief Create a cell which selects between the values of two
     * cells based on the value of a \a condition cell.
     *
     * If the value of the \a condition cell is \p true, the cell
     * evaluates to the value of the \a if_true cell. Otherwise the
     * cell evaluates to the value of the \a if_false cell.
     *
     * \param condition Condition cell used to select between \a
     *    if_true and \a if_false
     *
     * \param if_true Cell selected when \a condition is \p true
     *
     * \param if_false Cell selected when \a condition is \p false
     *
     * \return The selection cell
     */
    auto select(Cell auto condition, Cell auto if_true, Cell auto if_false) {
        return make_compute_cell([=] () {
            return condition.value() ? if_true.value() : if_false.value();
        }, condition, if_true, if_false);
    }

    /**
     * \brief Create a cell which evaluates to the value of another
     * cell only if a \a condition is \p true.
     *
     * If the value of the \a condition cell is \p true, the cell
     * evaluates to the value of the \a if_true cell. Otherwise the
     * cell preserves its current value as if by calling \p none().
     *
     * \param condition Condition cell
     *
     * \param if_true Cell selected when \a condition is \p true
     *
     * \return The selection cell
     */
    auto select(Cell auto condition, Cell auto if_true) {
        return store(
            make_compute_cell([=] () {
                if (!condition.value()) {
                    none();
                }

                return if_true.value();
            }, condition, if_true)
        );
    }

}  // live_cells

#endif /* LIVE_CELLS_BOOLEAN_HPP */
