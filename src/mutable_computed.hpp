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

#ifndef LIVE_CELLS_MUTABLE_COMPUTED_HPP
#define LIVE_CELLS_MUTABLE_COMPUTED_HPP

#include <tuple>
#include <utility>
#include <concepts>

#include "static_mutable_compute_cell.hpp"
#include "dynamic_mutable_compute_cell.hpp"
#include "util.hpp"

namespace live_cells {
    /**
     * Create a mutable computed cell.
     *
     * This differs from the other overload in that the argument cells
     * are not specified explicitly, but are determined
     * dynamically.
     *
     * The compute value function @a compute should use the `use()`
     * function to retrieve the values of its arguments.
     *
     * @param fn Compute value function.
     *
     *   This function is called with no arguments. The values of the
     *   argument cells should be referenced with `use()`.
     *
     * @param reverse Reverse computation function.
     *
     *   This function is passed the value that was assigned to the
     *   cell.
     *
     * @return The cell.
     */
    template <std::invocable F, typename R>
    auto mutable_computed(F&& fn, R&& reverse) {
        return dynamic_mutable_compute_cell<F,R>(
            std::forward<F>(fn),
            std::forward<R>(reverse)
        );
    }

    /**
     * Create a mutable computed cell.
     *
     * Ordinarily the value of the cell is the value computed by @a fn
     * which is a function of the cells in @a args.
     *
     * The returned cell is also a mutable cell, which means its value
     * can be set explicitly. When the value of the cell is set
     * explicitly, the reverse computation function @a reverse is
     * called. The function @a reverse, should set the values of the
     * cells in @a args such that the compute value function @a fn
     * returns the same value as the value that was assigned to the
     * cell.
     *
     * @param arg1 First argument to compute value function.
     *
     * @param arg2, args Remaining compute value function arguments
     *    followed by compute value and reverse compute functions.
     *
     *   The compute value function is passed the values of the
     *   argument cells, in the same order as they are provided in the
     *   argument list.
     *
     *   The reverse compute function (last in the argument list) is
     *   passed the value that was assigned to the cell.
     *
     * @return The cell.
     */
    template <typename A1, typename A2, typename... As>
    auto mutable_computed(A1 arg1, A2 arg2, As... args) {
        auto packed = internal::pack<2>(arg1, arg2, args...);

        auto fn_args = std::get<0>(packed);
        auto compute = std::get<1>(packed);
        auto reverse = std::get<2>(packed);

        return std::apply([&] (auto... args) {
            auto fn = [=] {
                return compute(args.value()...);
            };

            return make_mutable_compute_cell(fn, reverse, args...);
        }, fn_args);
    }

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_COMPUTED_HPP */
