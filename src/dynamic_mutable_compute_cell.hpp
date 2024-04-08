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

#ifndef LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP
#define LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP

#include <functional>
#include <memory>
#include <concepts>
#include <type_traits>

#include "keys.hpp"
#include "mutable_compute_cell_state.hpp"

namespace live_cells {

    /**
     * \brief Maintains the state of a \p
     * dynamic_mutable_compute_cell.
     */
    template<std::invocable F, typename R, typename value_type = std::invoke_result_t<F>>
    class dynamic_mutable_compute_cell_state : public mutable_compute_cell_state<value_type> {

        /** \brief Shorthand for parent class */
        typedef mutable_compute_cell_state<value_type> parent;

    public:

        /**
         * \brief Create the state for a dynamic mutable computed cell.
         *
         * \param key       Key identifying cell
         * \param compute   Value computation function
         * \param reverse   Reverse computation function
         */
        template <typename T, typename U>
        dynamic_mutable_compute_cell_state(key_ref key, T&& compute, U&& reverse) :
            parent(key, {}),
            compute_fn(std::forward<T>(compute)),
            reverse_fn(std::forward<U>(reverse)) {}

    protected:
        value_type compute() override {
            auto t = argument_tracker::global().with_tracker([this] (auto cell) {
                if (!this->arguments.count(cell)) {
                    this->arguments.emplace(cell);

                    cell.add_observer(this->observer_ptr());
                }
            });

            return compute_fn();
        }

        void reverse_compute(value_type value) override {
            reverse_fn(value);
        }

    private:
        /** \brief Value computation function */
        F compute_fn;

        /** \brief Reverse computation function */
        R reverse_fn;

    };

    /**
     * \brief A mutable computed cell with the argument cells
     * determined dynamically
     */
    template <std::invocable F, typename R>
    class dynamic_mutable_compute_cell :
        public stateful_cell<dynamic_mutable_compute_cell_state<F,R>> {

        /** \brief Shorthand for parent class */
        typedef stateful_cell<dynamic_mutable_compute_cell_state<F,R>> parent;

    public:

        /**
         * \brief Shorthand for the type of value held by this cell.
         */
        typedef std::invoke_result_t<F> value_type;

        /**
         * \brief Create a dynamic mutable computed cell.
         *
         * \param k Key identifying cell
         *
         * \param compute Compute value function.\n
         *   This function should compute the cell's value as a
         *   function of the cells in \a args.
         *
         * \param reverse Reverse computation function.\n
         *   This function should set the values of the cells in \a
         *   args, such that \a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         */
        template <typename T, typename U>
        dynamic_mutable_compute_cell(key_ref k, T&& compute, U&& reverse) :
            parent(k, std::forward<T>(compute), std::forward<U>(reverse)) {}

        /**
         * \brief Create a dynamic mutable computed cell.
         *
         * \param compute Compute value function.\n
         *   This function should compute the cell's value as a
         *   function of the cells in \a args.
         *
         * \param reverse Reverse computation function.\n
         *   This function should set the values of the cells in \a
         *   args, such that \a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         */
        template <typename T, typename U>
        dynamic_mutable_compute_cell(T&& compute, U&& reverse) :
            parent(key_ref::create<unique_key>(), std::forward<T>(compute), std::forward<U>(reverse)) {}

        value_type value() const {
            return this->state->value();
        }

        /**
         * \brief Set the value of the cell.
         *
         * This method calls the reverse computation function of the
         * cell.
         *
         * \note This method is marked const to allow the value of the
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

}  // live_cells

#endif /* LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP */
