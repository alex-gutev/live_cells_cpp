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

#include <concepts>
#include <type_traits>

#include "dependent_cell.hpp"
#include "make_cell.hpp"

namespace live_cells {

    /**
     * \brief A cell with a value that is a function of one or more argument
     * cells.
     *
     * The value computation function is provided to this class on
     * construction.
     *
     * \note This class is not intended to be instantiated directly by
     * users of the library. Instead create a computed cell with the
     * function \a computed.
     *
     * \note This is not a complete cell type and is such it does not
     * satisfy the \p Cell concept.
     */
    template <std::invocable F, Cell... Ts>
    class compute_cell_base : public dependent_cell<Ts...> {
    public:
        /**
         * \brief Create a cell with a value that is a function of the
         * values of the cells \a args.
         *
         * \param compute A function of no arguments, that is called
         *   to compute the cell's value when necessary.
         *
         * \param args Argument cells referenced by \a compute
         */
        compute_cell_base(F compute, Ts... args) :
            dependent_cell<Ts...>(args...),
            compute(compute) {}

        /**
         * \brief Get the value of the cell.
         *
         * \note The value of the cell is recomputed whenever it this
         * method is called.
         *
         * \return The cell's value
         */
        auto value() const {
            return compute();
        }

    private:
        /** \brief Compute value function */
        const F compute;

    };

    /**
     * \brief A computed cell with a static argument list.
     *
     * This cell is stateless in that it does not cache its value, nor
     * keeps track of its own observers. Instead, its value is
     * computed whenever \p value() is called and observers added to
     * this cell are added directly to the argument cells.
     */
    template <std::invocable F, Cell... Ts>
    using compute_cell = make_cell<compute_cell_base<F,Ts...>>;

    /**
     * Create a stateless computed cell with a given compute value
     * function and argument cells.
     *
     * @param f A function of no arguments, called to compute the
     *    value of the cell.
     *
     * @param args List of argument Cells on which the value of
     *   this cell depends.
     *
     * @return The computed cell.
     */
    auto make_compute_cell(std::invocable auto f, auto... args) {
        return compute_cell<decltype(f), decltype(args)...>(f, args...);
    }

    /**
     * Create a stateless computed cell with a given compute value
     * function and argument cells.
     *
     * @param key Key identifying the cell.
     *
     * @param f A function of no arguments, called to compute the
     *    value of the cell.
     *
     * @param args List of argument Cells on which the value of
     *   this cell depends.
     *
     * @return The computed cell.
     */
    auto make_compute_cell(key_ref key, std::invocable auto f, auto... args) {
        return compute_cell<decltype(f), decltype(args)...>(key, f, args...);
    }

}  // live_cells

#endif /* LIVE_CELLS_COMPUTE_CELL_HPP */
