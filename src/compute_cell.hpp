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
