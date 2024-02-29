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

#ifndef LIVE_CELLS_MUTABLE_CELL_VIEW_HPP
#define LIVE_CELLS_MUTABLE_CELL_VIEW_HPP

#include <functional>
#include <tuple>
#include <utility>

#include "compute_cell.hpp"
#include "mutable_cell.hpp"

namespace live_cells {
    /**
     * A stateless mutable computed cell that does not hold its own
     * value, nor tracks observers.
     */
    template <typename T, typename... Os>
    class mutable_cell_view : public compute_cell<T, Os...> {

        /** Shorthand for parent class */
        typedef compute_cell<T, Os...> parent;

        /** Compute function type */
        typedef std::function<T()> compute_fn;

        /** Reverse function type */
        typedef std::function<void(const T&)> reverse_fn;

    public:
        using parent::value;

        /**
         * Create a stateless mutable computed cell.
         *
         * @param compute Compute value function.
         *
         *   This function should compute the cell's value as a
         *   function of the cells in @a args.
         *
         * @param reverse Reverse computation function.
         *
         *   This function should set the values of the cells in @a
         *   args, such that @a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         *
         * @param args Arguments to @a compute
         */
        mutable_cell_view(compute_fn compute, reverse_fn reverse, Os... args) :
            parent(compute, args...),
            reverse(reverse) {}


        /**
         * Create a stateless mutable computed cell.
         *
         * @param key Key identifying the cell.
         *
         * @param compute Compute value function.
         *
         *   This function should compute the cell's value as a
         *   function of the cells in @a args.
         *
         * @param reverse Reverse computation function.
         *
         *   This function should set the values of the cells in @a
         *   args, such that @a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         *
         * @param args Arguments to @a compute
         */
        mutable_cell_view(key_ref key, compute_fn compute, reverse_fn reverse, Os... args) :
            parent(key, compute, args...),
            reverse(reverse) {}

        /**
         * Set the cell's value to @a value.
         *
         * @param value The value to which to set the cell.
         */
        void value(T value) {
            batch([&] {
                reverse(value);
            });
        }

    private:
        const reverse_fn reverse;
    };

    /**
     * Namespace for private definitions
     */
    namespace internal {
        template <typename C, typename R, typename... As>
        auto make_mutable_cell_view(C compute, R reverse, As... args) {
            typedef decltype(compute(args.value()...)) value_type;

            auto fn = [=] {
                return compute(args.value()...);
            };

            return mutable_cell_view<value_type, As...>(fn, reverse, args...);
        }

        template <typename C, typename R, typename... As>
        auto make_mutable_cell_view(key_ref key, C compute, R reverse, As... args) {
            typedef decltype(compute(args.value()...)) value_type;

            auto fn = [=] {
                return compute(args.value()...);
            };

            return mutable_cell_view<value_type, As...>(key, fn, reverse, args...);
        }
    }  // internal

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

        return internal::unpack([&] (auto... args) {
            return internal::make_mutable_cell_view(key, compute, reverse, args...);
        }, fn_args);
    }

    template <typename A1, typename A2, typename... As>
    auto cell_view(A1 arg1, A2 arg2, As... args) {
        auto packed = internal::pack<2>(arg1, arg2, args...);

        auto fn_args = std::get<0>(packed);
        auto compute = std::get<1>(packed);
        auto reverse = std::get<2>(packed);

        return internal::unpack([&] (auto... args) {
            return internal::make_mutable_cell_view(compute, reverse, args...);
        }, fn_args);
    }

}  // live_cells


#endif /* LIVE_CELLS_MUTABLE_CELL_VIEW_HPP */
