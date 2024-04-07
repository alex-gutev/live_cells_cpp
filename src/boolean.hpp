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
     * \brief reate a cell which evaluates to the value of another
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
