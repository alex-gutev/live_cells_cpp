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

#ifndef LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP
#define LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP

#include <functional>
#include <memory>
#include <concepts>
#include <type_traits>

#include "keys.hpp"
#include "mutable_compute_cell_state.hpp"
#include "changes_only_state.hpp"

namespace live_cells {

    /**
     * \brief Maintains the state for a \p
     * static_mutable_compute_cell.
     */
    template <std::invocable F, typename R>
    class static_mutable_compute_cell_state : public mutable_compute_cell_state<std::invoke_result_t<F>> {
    protected:
        typedef std::invoke_result_t<F> value_type;

        /** Shorthand for parent class */
        typedef mutable_compute_cell_state<value_type> parent;

    public:

        /**
         * \brief Create the state for a static_mutable_compute cell.
         *
         * \param key       Key identifying cell
         * \param compute   Value computation function
         * \param reverse   Reverse computation function
         * \param arguments Set of argument cells referenced in \a compute
         */
        template <typename T, typename U>
        static_mutable_compute_cell_state(key_ref key, T&& compute, U&& reverse, const std::unordered_set<cell> &arguments) :
            parent(key, arguments),
            compute_fn(std::forward<T>(compute)),
            reverse_fn(std::forward<U>(reverse)) {}

    protected:
        /**
         * \brief Compute the value of the cell as a function of the
         * arguments.
         *
         * \return The computed value.
         */
        value_type compute() override {
            return compute_fn();
        }

        /**
         * \brief Call the reverse computation function of the cell.
         *
         * \param value The value assigned to the cell.
         */
        void reverse_compute(value_type value) override {
            reverse_fn(value);
        }

        void init() override {
            parent::init();

            for (auto arg : this->arguments) {
                arg.add_observer(this->observer_ptr());
            }
        }

    private:
        /** \brief Value computation function */
        const F compute_fn;

        /** \brief Reverse computation function */
        const R reverse_fn;

    };


    /**
     * \brief A \p static_mutable_compute_cell_state that only
     * notifies the observers of the cell when the cell's value has
     * actually changed.
     */
    template <std::invocable F, typename R>
    class static_mutable_compute_changes_only_cell_state :
        public changes_only_cell_state<static_mutable_compute_cell_state<F,R>> {

        typedef changes_only_cell_state<static_mutable_compute_cell_state<F,R>> parent;

    public:
        using parent::parent;
    };

    /**
     * \brief A mutable computed cell with arguments determined at
     * compile-time.
     */
    template <std::invocable F, typename R, typename State = static_mutable_compute_cell_state<F,R>>
    class static_mutable_compute_cell : public stateful_cell<State> {

        /** Shorthand for parent class */
        typedef stateful_cell<State> parent;

    public:
        /**
         * \brief The type of value held by this cell.
         */
        typedef std::invoke_result_t<F> value_type;

        /**
         * \brief Create a static mutable computed cell.
         *
         * \param k Key identifying cell
         *
         * \param compute Compute value function.\n
         *   This function is called with no arguments to compute the
         *   value of the cell.
         *
         * \param reverse Reverse computation function.\n
         *   This function should set the values of the cells in \a
         *   args, such that \a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         *
         * \param args Arguments to \a compute
         */
        template <typename T, typename U, typename... Args>
        static_mutable_compute_cell(key_ref k, T&& compute, U&& reverse, Args&&... args) :
            parent(k,
                   std::forward<T>(compute),
                   std::forward<U>(reverse),
                   std::unordered_set<cell>({cell(args)...})) {}

        /**
         * \brief Create a static mutable computed cell.
         *
         * \param compute Compute value function.\n
         *   This function is called with no arguments to compute the
         *   value of the cell.
         *
         * \param reverse Reverse computation function.\n
         *   This function should set the values of the cells in \a
         *   args, such that \a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         *
         * \param args Arguments to \a compute
         */
        template <typename T, typename U, typename... Args>
        static_mutable_compute_cell(T&& compute, U&& reverse, Args&&... args) :
            parent(key_ref::create<unique_key>(), std::forward<T>(compute), std::forward<U>(reverse), std::unordered_set<cell>({cell(args)...})) {}

        /**
         * \brief Get the value of the cell.
         *
         * \return The value.
         */
        value_type value() const {
            return this->state->value();
        }

        /**
         * \brief Set the value of the cell.
         *
         * This method triggers the reverse computation function of
         * the cell.
         *
         * \note This method is marked const to allow the value of the
         * cell to be set when it is copy-captured by a lambda.
         *
         * \param value The new value
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

        /**
         * \brief Get the value of the cell and track it as a
         * dependency.
         *
         * \return The value.
         */
        value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }
    };

    /**
     * \brief Create a \p static_mutable_compute_cell with compute
     * function \a compute, reverse compute function \a reverse and
     * arguments \a args.
     *
     * \param compute Compute value function.
     * \param reverse Reverse compute function.
     * \param args    Argument cells
     *
     * \return The cell
     */
    template <std::invocable C, typename R, Cell... As>
    auto make_mutable_compute_cell(C compute, R reverse, As... args) {
        return static_mutable_compute_cell<C,R>(compute, reverse, args...);
    }

    /**
     * \brief Create a \p static_mutable_compute_cell with compute
     * function \a compute, reverse compute function \a reverse and
     * arguments \a args, that only notifies its observers when its
     * value has actually changed.
     *
     * \note The difference between this and the other overloads, is
     * that with this overload, the cell only notifies its observers
     * when its new value is not equal to its previous value.
     *
     * \param option Changes only cell option
     * \param compute Compute value function.
     * \param reverse Reverse compute function.
     * \param args    Argument cells
     *
     * \return The cell
     */
    template <std::invocable C, typename R, Cell... As>
    auto make_mutable_compute_cell(changes_only_option option, C compute, R reverse, As... args) {
        typedef static_mutable_compute_changes_only_cell_state<C,R> Base;

        return static_mutable_compute_cell<C,R,Base>(compute, reverse, args...);
    }

    /**
     * \brief Create a \p static_mutable_compute_cell with compute
     * function \a compute, reverse compute function \a reverse and
     * arguments \a args.
     *
     * \param key     Key identifying the cell
     * \param compute Compute value function.
     * \param reverse Reverse compute function.
     * \param args    Argument cells
     *
     * \return The cell
     */
    template <std::invocable C, typename R, Cell... As>
    auto make_mutable_compute_cell(key_ref key, C compute, R reverse, As... args) {
        return static_mutable_compute_cell<C,R>(key, compute, reverse, args...);
    }

    /**
     * \brief Create a \p static_mutable_compute_cell with compute
     * function \a compute, reverse compute function \a reverse and
     * arguments \a args, that only notifies its observers when its
     * value has actually changed.
     *
     * \note The difference between this and the other overloads, is
     * that with this overload, the cell only notifies its observers
     * when its new value is not equal to its previous value.
     *
     * \param option  Changes only cell option
     * \param key     Key identifying the cell
     * \param compute Compute value function.
     * \param reverse Reverse compute function.
     * \param args    Argument cells
     *
     * \return The cell
     */
    template <std::invocable C, typename R, Cell... As>
    auto make_mutable_compute_cell(changes_only_option option, key_ref key, C compute, R reverse, As... args) {
        typedef static_mutable_compute_changes_only_cell_state<C,R> Base;

        return static_mutable_compute_cell<C,R,Base>(key, compute, reverse, args...);
    }

}  // live_cells

#endif /* LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP */
