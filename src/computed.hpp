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

#ifndef LIVE_CELLS_COMPUTED_HPP
#define LIVE_CELLS_COMPUTED_HPP

#include <tuple>
#include <utility>

#include "compute_cell.hpp"
#include "dynamic_compute_cell.hpp"
#include "exceptions.hpp"
#include "util.hpp"

namespace live_cells {

    /**
     * \brief Create a cell with a value that is a function of one or
     * more argument cells.
     *
     * The argument cells, referenced within \a compute, are
     * determined dynamically. For this to work, the values of the
     * argument cells must be referenced using the function call
     * operator and not the \p Cell::value() getter method.
     *
     * \param compute Compute value function
     *
     * \return A computed cell
     */
    template <std::invocable F>
    auto computed(F&& compute) {
        return dynamic_compute_cell<F>(std::forward<F>(compute));
    }

    /**
     * \brief Create a cell with a value that is a function of one or
     * more argument cells and only notifies its observers when its
     * value has actually changed.
     *
     * The argument cells, referenced within \a compute, are
     * determined dynamically. For this to work, the values of the
     * argument cells must be referenced using the function call
     * operator and not the \p Cell::value() getter method.
     *
     * \note The difference between this and the other overloads, is
     * that with this overload, the cell only notifies its observers
     * when its new value is not equal to its previous value.
     *
     * \param option Changes only cell option
     *
     * \param compute Compute value function
     *
     * \return A computed cell
     */
    template <std::invocable F>
    auto computed(changes_only_option option, F&& compute) {
        return dynamic_compute_cell<F, dynamic_compute_changes_only_cell_state<F>>(
            std::forward<F>(compute)
        );
    }

    /**
     * \brief Create a cell with a value that is a function of one or more
     * argument cells.
     *
     * The argument cells, referenced within \a compute, are
     * determined dynamically. For this to work, the values of the
     * argument cells must be referenced using the function call
     * operator and not the \p Cell::value() getter method.
     *
     * \param key     Key identifying the cell
     * \param compute Compute value function
     *
     * \return A computed cell
     */
    template <std::invocable F>
    auto computed(key_ref key, F&& compute) {
        return dynamic_compute_cell<F>(key, std::forward<F>(compute));
    }

    /**
     * \brief Create a cell with a value that is a function of one or more
     * argument cells and only notifies its observers when its
     * value has actually changed.
     *
     * The argument cells, referenced within \a compute, are
     * determined dynamically. For this to work, the values of the
     * argument cells must be referenced using the function call
     * operator and not the \p Cell::value() getter method.
     *
     * \note The difference between this and the other overloads, is
     * that with this overload, the cell only notifies its observers
     * when its new value is not equal to its previous value.
     *
     * \param option Changes only cell option
     *
     * \param key     Key identifying the cell
     * \param compute Compute value function
     *
     * \return A computed cell
     */
    template <std::invocable F>
    auto computed(changes_only_option option, key_ref key, F&& compute) {
        return dynamic_compute_cell<F, dynamic_compute_changes_only_cell_state<F>>(
            key,
            std::forward<F>(compute)
        );
    }

    /**
     * \brief Create a cell with a value that is a function of the
     * values of one or more argument cells.
     *
     * \param arg1 First argument to value computation function.
     *
     * \param args Remaining arguments followed by compute value function.\n
     *   The compute value function is called to compute the value of
     *   the cell when necessary. The values of the argument cells are
     *   passed to the compute value function in the same order as
     *   they are provided in this function.
     *
     * \return A computed cell
     */
    template <typename A, typename... As>
    auto computed(A arg1, As... args) {
        auto packed = internal::pack<1>(arg1, args...);

        auto fn_args = std::get<0>(packed);
        auto fn = std::get<1>(packed);

        return std::apply([&] (auto... args) {
            auto f = [=] {
                return fn(args.value()...);
            };

            return make_compute_cell(f, args...);
        }, fn_args);
    }

    /**
     * \brief Create a cell with a value that is a function of the
     * values of one or more argument cells.
     *
     * \param key Key identifying the cell
     *
     * \param arg1 First argument to value computation function.
     *
     * \param args Remaining arguments followed by compute value function.\n
     *   The compute value function is called to compute the value of
     *   the cell when necessary. The values of the argument cells are
     *   passed to the compute value function in the same order as
     *   they are provided in this function.
     *
     * \return A computed cell
     */
    template <typename A, typename... As>
    auto computed(key_ref key, A arg1, As... args) {
        auto packed = internal::pack<1>(arg1, args...);

        auto fn_args = std::get<0>(packed);
        auto fn = std::get<1>(packed);

        return std::apply([&] (auto... args) {
            auto f = [=] {
                return fn(args.value()...);
            };

            return make_compute_cell(key, f, args...);
        }, fn_args);
    }

    /**
     * \brief Stop the computation of the current computed cell's
     * value.
     *
     * When this method is called within the value computation of a
     * cell, the cell's value is not recomputed. Instead its current
     * value is preserved.
     */
    inline void none() {
        throw stop_compute_exception();
    }

}  // live_cells

#endif /* LIVE_CELLS_COMPUTED_HPP */
