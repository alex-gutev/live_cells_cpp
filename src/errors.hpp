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

#ifndef LIVE_CELLS_ERRORS_HPP
#define LIVE_CELLS_ERRORS_HPP

#include "observable.hpp"
#include "computed.hpp"
#include "store_cell.hpp"

namespace live_cells {
    /**
     * Create a cell which handles all exceptions thrown while
     * computing the value of a cell.
     *
     * If an exception is thrown during the computation of the value
     * of `cell`, the cell evaluates to the value of the `error_value`
     * cell.
     *
     * @param cell        The cell for which to handle exceptions
     *
     * @param error_value The cell holding the value to use if an
     *    exception is thrown.
     *
     * @return The new cell
     */
    auto on_error(const Cell auto &cell, const Cell auto &error_value) {
        return store(
            make_compute_cell([=] () {
                try {
                    return cell.value();
                }
                catch (...) {
                    return error_value.value();
                }
            }, cell, error_value)
        );
    }

    /**
     * Like the other overload but only handles exceptions of type
     * `E`, which is the first template type parameter.
     *
     * Type `E` cannot be deduced so it should be specified
     * explicitly.
     */
    template <typename E>
    auto on_error(const Cell auto &cell, const Cell auto &error_value) {
        return store(
            make_compute_cell([=] () {
                try {
                    return cell.value();
                }
                catch (const E &) {
                    return error_value.value();
                }
            }, cell, error_value)
        );
    }

}  // live_cells

#endif /* LIVE_CELLS_ERRORS_HPP */
