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
