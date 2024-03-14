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

#ifndef LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP
#define LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP

#include <functional>
#include <memory>
#include <concepts>
#include <type_traits>

#include "keys.hpp"
#include "mutable_compute_cell_state.hpp"

namespace live_cells {

    /**
     * Maintains the state for a mutable computed cell with an
     * argument list determined at compile-time.
     */
    template <std::invocable F, typename R>
    class static_mutable_compute_cell_state : public mutable_compute_cell_state<std::invoke_result_t<F>> {
        typedef std::invoke_result_t<F> value_type;

        /** Shorthand for parent class */
        typedef mutable_compute_cell_state<value_type> parent;

    public:

        /**
         * Create the state for a static mutable computed cell.
         *
         * @param key       Key identifying cell
         * @param compute   Value computation function
         * @param reverse   Reverse computation function
         * @param arguments Set of argument cells referenced in @a compute
         */
        template <typename T, typename U>
        static_mutable_compute_cell_state(key_ref key, T&& compute, U&& reverse, const std::unordered_set<cell> &arguments) :
            parent(key, arguments),
            compute_fn(std::forward<T>(compute)),
            reverse_fn(std::forward<U>(reverse)) {}

    protected:
        value_type compute() override {
            return compute_fn();
        }

        void reverse_compute(value_type value) override {
            reverse_fn(value);
        }

    private:
        /** Value computation function */
        const F compute_fn;

        /** Reverse computation function */
        const R reverse_fn;

    };

    /**
     * A mutable computed cell with arguments determined at
     * compile-time.
     */
    template <std::invocable F, typename R>
    class static_mutable_compute_cell :
        public stateful_cell<static_mutable_compute_cell_state<F,R>> {

        /** Shorthand for parent class */
        typedef stateful_cell<static_mutable_compute_cell_state<F,R>> parent;

    public:
        typedef std::invoke_result_t<F> value_type;

        /**
         * Create a static mutable computed cell.
         *
         * @param k Key identifying cell
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
        template <typename T, typename U, typename... Args>
        static_mutable_compute_cell(key_ref k, T&& compute, U&& reverse, Args&&... args) :
            parent(k, std::forward<T>(compute), std::forward<U>(reverse), {std::forward<Args>(args)...}) {}

        template <typename T, typename U, typename... Args>
        static_mutable_compute_cell(T&& compute, U&& reverse, Args&&... args) :
            parent(key_ref::create<unique_key>(), std::forward<T>(compute), std::forward<U>(reverse), std::unordered_set<cell>({cell(args)...})) {}

        value_type value() const {
            return this->state->value();
        }

        /**
         * Set the value of the cell.
         *
         * This method triggers the reverse computation function of
         * the cell.
         *
         * NOTE: This method is marked const to allow the value of the
         * cell to be set when it is copy-captured by a lambda.
         */
        void value(value_type value) const {
            this->state->value(value);
        }

        value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }
    };


    /**
     * Create a static_mutable_compute_cell with compute function @a
     * compute, reverse compute function @a reverse and arguments @a
     * args.
     *
     * @param compute Compute value function.
     * @param reverse Reverse compute function.
     * @param args    Argument cells
     *
     * @return A static_mutable_compute_cell
     */
    template <std::invocable C, typename R, Cell... As>
    auto make_mutable_compute_cell(C compute, R reverse, As... args) {
        return static_mutable_compute_cell<C,R>(compute, reverse, args...);
    }

}  // live_cells

#endif /* LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP */
