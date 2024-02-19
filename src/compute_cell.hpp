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

#ifndef LIVE_CELLS_COMPUTE_CELL_HPP
#define LIVE_CELLS_COMPUTE_CELL_HPP

#include "dependent_cell.hpp"

namespace live_cells {

    /**
     * A cell with a value that is a function of one or more argument
     * cells.
     *
     * The value computation function is provided to this class on
     * construction.
     */
    template <typename T, typename... Ts>
    class compute_cell : public dependent_cell<T, Ts...> {
    public:
        /**
         * Create a cell with a value that is a function of the values
         * of the cells @a args.
         *
         * @param compute A function of no arguments, that is called
         *   to compute the cell's value when necessary.
         *
         * @param args Argument cell observables
         */
        compute_cell(std::function<T()> compute, Ts... args) :
            dependent_cell<T, Ts...>(args...),
            compute(compute) {}

        /**
         * Create a cell with a value that is a function of the values
         * of the cells @a args.
         *
         * @param key Key identifying cell
         *
         * @param compute A function of no arguments, that is called
         *   to compute the cell's value when necessary.
         *
         * @param args Argument cell observables
         */
        compute_cell(key_ref key, std::function<T()> compute, Ts... args) :
            dependent_cell<T, Ts...>(key, args...),
            compute(compute) {}

        T value() const override {
            return compute();
        }

    private:
        const std::function<T()> compute;

    };

}  // live_cells

#endif /* LIVE_CELLS_COMPUTE_CELL_HPP */
