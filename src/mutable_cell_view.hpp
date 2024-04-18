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

/** \file */

#ifndef LIVE_CELLS_MUTABLE_CELL_VIEW_HPP
#define LIVE_CELLS_MUTABLE_CELL_VIEW_HPP

#include <functional>
#include <tuple>
#include <utility>

#include "make_cell.hpp"
#include "compute_cell.hpp"
#include "mutable_cell.hpp"

namespace live_cells {
    /**
     * \brief A stateless mutable computed cell that does not cache
     * its own value.
     *
     * The value of the cell is computed whenever it is accessed and
     * observers are added directly to the argument cells.
     *
     * \warning This is not a complete cell and hence does not
     * satisfy the \p Cell concept. As such this class should not be
     * instantiated directly. Instead the \p cell_view()
     * function should be used to create stateless mutable computed
     * cells.
     */
    template <std::invocable F, typename R, typename... Os>
    class mutable_cell_view_base : public compute_cell_base<F, Os...> {

        /** \brief Shorthand for parent class */
        typedef compute_cell_base<F, Os...> parent;

        /** \brief Shorthand for the type of value held by this cell */
        typedef std::invoke_result_t<F> value_type;

    public:
        using parent::value;

        /**
         * \brief Create a stateless mutable computed cell.
         *
         * \param compute Compute value function.\n
         *   This function, of no arguments, should compute the cell's
         *   value as a function of the cells in \a args.
         *
         * \param reverse Reverse computation function.\n
         *   This function should set the values of the cells in \a
         *   args, such that \a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         *
         * \param args Argument cells referenced in \a compute
         */
        mutable_cell_view_base(F compute, R reverse, Os... args) :
            parent(compute, args...),
            reverse(reverse) {}

        /**
         * \brief Set the value of the cell.
         *
         * \param value The new value
         */
        void value(value_type value) const {
            batch([&] {
                reverse(value);
            });
        }

    private:
        /**
         * \brief Reverse computation function.
         */
        const R reverse;
    };

    /**
     * \brief A stateless mutable computed cell.
     *
     * This implements a mutable computed cell which does not cache
     * its value. Instead its value is computed on demand and
     * observers are added directly to the argument cells.
     *
     * \note This class satisfies the \p Cell and \p MutableCell
     * concepts.
     */
    template <std::invocable F, typename R, Cell... Ts>
    using mutable_cell_view = make_mutable_cell<mutable_cell_view_base<F,R,Ts...>>;

    /**
     * Create a stateless mutable computed cell with a given compute
     * and reverse function and argument cells.
     *
     * @param f A function of no arguments, called to compute the
     *    value of the cell.
     *
     * @param args List of argument Cells on which the value of
     *   this cell depends.
     *
     * @return The computed cell.
     */
    auto make_mutable_cell_view(std::invocable auto compute, auto reverse, auto... args) {
        return mutable_cell_view<
            decltype(compute),
            decltype(reverse),
            decltype(args)...>(
                compute,
                reverse,
                args...
        );
    }

    /**
     * Create a stateless computed cell with a given compute and
     * reverse function function and argument cells
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
    auto make_mutable_cell_view(key_ref key, std::invocable auto compute, auto reverse, auto... args) {
        return mutable_cell_view<
            decltype(compute),
            decltype(reverse),
            decltype(args)...>(
                key,
                compute,
                reverse,
                args...
        );
    }

    /**
     * Create a stateless mutable computed cell.
     *
     * This is like a mutable cell view but it does not store its own
     * value nor does it track its own observers. As a result this
     * cell has slightly different semantics from a mutable computed
     * cell, in that its value is always recomputed when accessed even
     * after it is set explicitly.
     *
     * @param key The key identifying the cell.
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
    auto cell_view(key_ref key, A1 arg1, A2 arg2, As... args) {
        auto packed = internal::pack<2>(arg1, arg2, args...);

        auto fn_args = std::get<0>(packed);
        auto compute = std::get<1>(packed);
        auto reverse = std::get<2>(packed);

        return std::apply([&] (auto... args) {
            auto f = [=] {
                return compute(args.value()...);
            };

            return make_mutable_cell_view(key, f, reverse, args...);
        }, fn_args);
    }

    template <typename A1, typename A2, typename... As>
    auto cell_view(A1 arg1, A2 arg2, As... args) {
        auto packed = internal::pack<2>(arg1, arg2, args...);

        auto fn_args = std::get<0>(packed);
        auto compute = std::get<1>(packed);
        auto reverse = std::get<2>(packed);

        return std::apply([&] (auto... args) {
            auto f = [=] {
                return compute(args.value()...);
            };

            return make_mutable_cell_view(f, reverse, args...);
        }, fn_args);
    }

}  // live_cells


#endif /* LIVE_CELLS_MUTABLE_CELL_VIEW_HPP */
