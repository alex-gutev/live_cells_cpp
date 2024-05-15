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
     * \brief Create a mutable computed cell with dynamically
     * determined argument cells.
     *
     * Ordinarily the value of the cell is the value computed by \a
     * fn as a function of other cells.
     *
     * The returned cell is also a mutable cell, which means its value
     * can be set explicitly. When the value of the cell is set
     * explicitly, the reverse computation function \a reverse is
     * called. \a reverse should set the values of the cells
     * referenced in \a fn such that \a fn returns the same value as
     * the value that was assigned to the cell.
     *
     * \attention The values of Cell's should be referenced in \a fn
     * using the function call operator, so that they are tracked as
     * dependencies of the mutable computed cell, and not using the \p
     * Cell::value() getter method.
     *
     * \note This differs from the other overload in that the argument cells
     * are not specified explicitly, but are determined
     * dynamically.
     *
     * \param fn Compute value function.\n
     *   This function is called with no arguments.
     *
     * \param reverse Reverse computation function.\n
     *   This function is passed the value that was assigned to the
     *   cell.
     *
     * \return The mutable computed cell
     */
    template <std::invocable F, typename R>
    auto mutable_computed(F&& fn, R&& reverse) {
        return dynamic_mutable_compute_cell<F,R>(
            std::forward<F>(fn),
            std::forward<R>(reverse)
        );
    }

    /**
     * \brief Create a mutable computed cell with dynamically
     * determined argument cells that only notifies its observers when
     * its value has actually changed.
     *
     * Ordinarily the value of the cell is the value computed by \a
     * fn as a function of other cells.
     *
     * The returned cell is also a mutable cell, which means its value
     * can be set explicitly. When the value of the cell is set
     * explicitly, the reverse computation function \a reverse is
     * called. \a reverse should set the values of the cells
     * referenced in \a fn such that \a fn returns the same value as
     * the value that was assigned to the cell.
     *
     * \attention The values of Cell's should be referenced in \a fn
     * using the function call operator, so that they are tracked as
     * dependencies of the mutable computed cell, and not using the \p
     * Cell::value() getter method.
     *
     * \note This differs from the other overload in that the argument cells
     * are not specified explicitly, but are determined
     * dynamically.
     *
     * \note With this overload, the cell only notifies its observers
     * when its new value is not equal to its previous value.
     *
     * \param fn Compute value function.\n
     *   This function is called with no arguments.
     *
     * \param reverse Reverse computation function.\n
     *   This function is passed the value that was assigned to the
     *   cell.
     *
     * \return The mutable computed cell
     */
    template <std::invocable F, typename R>
    auto mutable_computed(changes_only, F&& fn, R&& reverse) {
        return dynamic_mutable_compute_cell<F,R, dynamic_mutable_compute_changes_only_cell_state<F,R>>(
            std::forward<F>(fn),
            std::forward<R>(reverse)
        );
    }

    /**
     * \brief Create a mutable computed cell.
     *
     * Ordinarily the value of the cell is the value computed by a
     * function of the cells in \a args.
     *
     * The returned cell is also a mutable cell, which means its value
     * can be set explicitly. When the value of the cell is set
     * explicitly, the reverse computation function is called. The
     * reverse computation function, should set the values of the
     * cells in \a args such that the compute value function returns
     * the same value as the value that was assigned to the cell.
     *
     * \param arg1 First argument to compute value function.
     *
     * \param arg2, args Remaining compute value function arguments
     *    followed by compute value and reverse compute functions.\n\n
     *   The compute value function is passed the values of the
     *   argument cells, in the same order as they are provided in the
     *   argument list.\n\n
     *   The reverse compute function (last in the argument list) is
     *   passed the value that was assigned to the cell.
     *
     * \return The mutable computed cell.
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

    /**
     * \brief Create a mutable computed cell that only notifies its
     * observers when its value has actually changed.
     *
     * Ordinarily the value of the cell is the value computed by a
     * function of the cells in \a args.
     *
     * The returned cell is also a mutable cell, which means its value
     * can be set explicitly. When the value of the cell is set
     * explicitly, the reverse computation function is called. The
     * reverse computation function, should set the values of the
     * cells in \a args such that the compute value function returns
     * the same value as the value that was assigned to the cell.
     *
     * \note The difference between this and the other overloads, is
     * that with this overload, the cell only notifies its observers
     * when its new value is not equal to its previous value.
     *
     * \param arg1 First argument to compute value function.
     *
     * \param arg2, args Remaining compute value function arguments
     *    followed by compute value and reverse compute functions.\n\n
     *   The compute value function is passed the values of the
     *   argument cells, in the same order as they are provided in the
     *   argument list.\n\n
     *   The reverse compute function (last in the argument list) is
     *   passed the value that was assigned to the cell.
     *
     * \return The mutable computed cell.
     */
    template <typename A1, typename A2, typename... As>
    auto mutable_computed(changes_only, A1 arg1, A2 arg2, As... args) {
        auto packed = internal::pack<2>(arg1, arg2, args...);

        auto fn_args = std::get<0>(packed);
        auto compute = std::get<1>(packed);
        auto reverse = std::get<2>(packed);

        return std::apply([&] (auto... args) {
            auto fn = [=] {
                return compute(args.value()...);
            };

            return make_mutable_compute_cell(changes_only(), fn, reverse, args...);
        }, fn_args);
    }

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_COMPUTED_HPP */
