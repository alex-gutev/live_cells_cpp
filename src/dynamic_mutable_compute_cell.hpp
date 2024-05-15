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

#ifndef LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP
#define LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP

#include <functional>
#include <memory>
#include <concepts>
#include <type_traits>

#include "keys.hpp"
#include "mutable_compute_cell_state.hpp"
#include "changes_only_state.hpp"

namespace live_cells {

    /**
     * \brief Maintains the state of a \p
     * dynamic_mutable_compute_cell.
     */
    template<std::invocable F, typename R, typename ValueType = std::invoke_result_t<F>>
    class dynamic_mutable_compute_cell_state : public mutable_compute_cell_state<ValueType> {
    protected:
        typedef ValueType value_type;

        /** \brief Shorthand for parent class */
        typedef mutable_compute_cell_state<ValueType> parent;

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
        ValueType compute() override {
            auto t = argument_tracker::global().with_tracker([this] (auto cell) {
                if (!this->arguments.count(cell)) {
                    this->arguments.emplace(cell);

                    cell.add_observer(this->observer_ptr());
                }
            });

            return compute_fn();
        }

        void reverse_compute(ValueType value) override {
            reverse_fn(value);
        }

    private:
        /** \brief Value computation function */
        F compute_fn;

        /** \brief Reverse computation function */
        R reverse_fn;

    };

    /**
     * \brief A \p dynamic_mutable_compute_cell_state that only
     * notifies the observers of the cell if its value has actually
     * changed.
     */
    template <std::invocable F, typename R>
    class dynamic_mutable_compute_changes_only_cell_state :
        public changes_only_cell_state<dynamic_mutable_compute_cell_state<F,R>> {

        typedef changes_only_cell_state<dynamic_mutable_compute_cell_state<F,R>> parent;

    public:
        using parent::parent;
    };

    /**
     * \brief A mutable computed cell with the argument cells
     * determined dynamically
     */
    template <std::invocable F, typename R, typename State = dynamic_mutable_compute_cell_state<F,R>>
    class dynamic_mutable_compute_cell : public stateful_cell<State> {

        /** \brief Shorthand for parent class */
        typedef stateful_cell<State> parent;

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

        /**
         * \brief Set the value of the cell to \a value.
         *
         * \note Implemented using \p C::value(value).
         *
         * This method triggers the reverse computation function of
         * the cell.
         *
         * \param value The new value of the cell
         *
         * \return \a value
         */
        value_type operator=(const value_type &value) {
            this->value(value);
            return value;
        }

        /**
         * \brief Set the value of the cell.
         *
         * This is equivalent to \prop value(value).
         *
         * This method triggers the reverse computation function of
         * the cell.
         *
         * \note This \c const version is provided to allow setting
         * the value of a mutable cell within a lambda provided for
         * the reverse computation function of a mutable computed
         * cell.
         *
         * \param value The new value
         *
         * \return \a value
         */
        value_type operator=(const value_type &value) const {
            this->value(value);
            return value;
        }

        value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP */
