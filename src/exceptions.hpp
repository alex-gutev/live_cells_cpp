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

#ifndef LIVE_CELLS_EXCEPTIONS_HPP
#define LIVE_CELLS_EXCEPTIONS_HPP

#include <exception>

namespace live_cells {

    /**
     * Indicates that the value of a cell should not be computed.
     *
     * When this exception is thrown inside a cell's value computation
     * function, the cell's value is not updated. Instead it's current
     * value is preserved.
     */
    struct stop_compute_exception : std::exception {
        const char *what() const noexcept override {
            return "Stop computation of the value of a cell.";
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_EXCEPTIONS_HPP */
