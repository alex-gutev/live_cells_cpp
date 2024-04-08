/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
