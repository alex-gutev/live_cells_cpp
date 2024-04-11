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

#ifndef LIVE_CELLS_MUTABLE_CELL_HPP
#define LIVE_CELLS_MUTABLE_CELL_HPP

#include <memory>

#include "cell_state.hpp"
#include "stateful_cell.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * \brief Defer changes to the values of mutable cells.
     *
     * When an instance of this class is in scope, observers of
     * mutable cells will not be notified immediately when the values
     * of the cells are set, but instead will be notified when this
     * object is destroyed.
     *
     * Creating a new instance when one already exists has no effect,
     * the observers will only be notified when the first instance is
     * destroyed.
     *
     * \warning Do not allocate instances of this class using dynamic
     * allocation.
     */
    class batch_update {
    public:
        batch_update();
        ~batch_update();

        batch_update(const batch_update &) = delete;
        batch_update& operator=(const batch_update &) = delete;

    private:
        /** Is this instance responsible for notifying the observers */
        bool is_batching;

        /**
         * Is a batch update currently in effect?
         */
        static bool is_batch_update();

        /**
         * Add a mutable cell state to the list of batched updates.
         *
         * The observers of the cell associated with the state will be
         * notified with cell_state::notify_update() when the values
         * of all the cells in the batch have been set.
         */
        static void add_to_batch(std::shared_ptr<cell_state> state);

        template <typename T>
        friend class mutable_cell_state;
    };

    /**
     * \brief Maintains the state of a \p mutable_cell.
     */
    template<typename T>
    class mutable_cell_state : public cell_state {
    public:
        using cell_state::cell_state;

        /**
         * \brief Create a mutable cell state with the value
         * initialized to \a value.
         *
         * \param key    Key identifying the state
         * \param value  Initial value
         */
        mutable_cell_state(key_ref key, T value) :
            cell_state(key),
            value_(value) {
        }

        /**
         * \brief Get the value of the cell.
         *
         * \return The value
         */
        T value() {
            return value_;
        }

        /**
         * \brief Set the value of the cell and notify its observers.
         *
         * \param value The new value.
         */
        void value(T value) {
            if (value_ != value) {
                notify_will_update();
                value_ = std::move(value);

                if (!batch_update::is_batch_update()) {
                    notify_update();
                }
                else {
                    batch_update::add_to_batch(this->shared_from_this());
                }
            }
        }

    protected:
        /**
         * \brief Set the cell's value without notifying observers.
         *
         * \param value The new value
         */
        void silent_set(T value) {
            value_ = std::move(value);
        }

        /**
         * \brief Is a batch update of mutable cells currently in effect?
         *
         * \return \p true if a batch update is in effect
         */
        static bool is_batch_update() {
            return batch_update::is_batch_update();
        }

        /**
         * \brief Add this state to the list of mutable cells that
         * were updating during the current batch update.
         */
        void add_to_batch() {
            batch_update::add_to_batch(this->shared_from_this());
        }

    private:
        /** \brief The cell's value */
        T value_;
    };

    /**
     * \brief A stateful cell which can have its value set directly.
     */
    template<typename T>
    class mutable_cell : public stateful_cell<mutable_cell_state<T>> {
        /**
         * \brief Shorthand for parent class
         */
        typedef stateful_cell<mutable_cell_state<T>> parent;

    public:
        /**
         * \brief The type of value held by this cell
         */
        typedef T value_type;

        /**
         * \brief Create a mutable cell with a default initialized
         * value.
         */
        mutable_cell() : parent(key_ref::create<unique_key>()) {}

        /**
         * \brief Create a mutable cell with an initial value.
         *
         * \param value The initial value of the cell
         */
        mutable_cell(T value) :
            mutable_cell(key_ref::create<unique_key>(), value) {}

        /**
         * \brief Create a mutable cell with an initial value
         *
         * If there is no state associated with key \a key, a new
         * state is created and the value of the cell is initialized
         * to \a value.
         *
         * If there is a state associated with \a key, \a value is
         * ignored.
         *
         * \param key   Key identifying cell
         * \param value Initial value
         */
        mutable_cell(key_ref key, T value) :
            parent(key, value) {
        }


        /**
         * \brief Set the value of the cell.
         *
         * This is equivalent to \prop value(value).
         *
         * \param value The new value
         *
         * \return \a value
         */
        T operator=(const T &value) {
            this->value(value);
            return value;
        }

        /**
         * \brief Set the value of the cell.
         *
         * This is equivalent to \prop value(value).
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
        T operator=(const T &value) const {
            this->value(value);
            return value;
        }

        /**
         * \brief Get the value of the cell.
         *
         * \return The value of the cell.
         */
        T value() const {
            return this->state->value();
        }

        /**
         * \brief Set the value of the cell.
         *
         * \note This method is marked const to allow the value of the
         * cell to be set when it is copy-captured by a lambda.
         *
         * \param value The new value
         */
        void value(T value) const {
            this->state->value(value);
        }

        /**
         * \brief Get the value of the cell and track it as a
         * dependency.
         *
         * \return The value of the cell.
         */
        T operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }
    };

    /**
     * \brief Create a new mutable cell with an initial value.
     *
     * \param value The initial value
     *
     * \return A mutable cell
     */
    template<typename T>
    mutable_cell<T> variable(T value) {
        return mutable_cell<T>(value);
    }

    /**
     * \brief Create a new mutable cell, identified by a key, with an
     * initial value.
     *
     * \note \a value is ignored if \a key already identifies a
     * mutable cell.
     *
     * \param key   Key identifying cell
     * \param value The initial value
     *
     * \return A mutable cell
     */
    template<typename T>
    mutable_cell<T> variable(key_ref key, T value) {
        return mutable_cell<T>(key, value);
    }

    /**
     * \brief Batch changes to the values of mutable cells.
     *
     * The function \a fn is called with zero arguments. When the
     * value of a mutable cell is set within \a fn, the observers of
     * the cell are only notified of the change after \a fn returns.
     *
     * \param fn A function of zero arguments
     */
    template<typename F>
    void batch(F fn) {
        batch_update b;
        fn();
    }

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_CELL_HPP */
