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

#ifndef LIVE_CELLS_STORE_CELL_HPP
#define LIVE_CELLS_STORE_CELL_HPP

#include "keys.hpp"
#include "compute_state.hpp"
#include "stateful_cell.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * Key identifying a store_cell
     */
    template <typename T>
    struct store_cell_key : value_key<T> {
        using value_key<T>::value_key;
    };

    /** Forward declaration */
    template <Cell C>
    class store_cell_state;

    /**
     * Defines the computation function of the state of a store_cell.
     */
    template <Cell C>
    class store_cell_compute_state {
        /**
         * Argument cell
         */
        const C arg;

        friend class store_cell_state<C>;

    public:
        store_cell_compute_state(C arg) :
            arg(arg) {}

        C::value_type operator()(observer::ref) {
            return arg.value();
        }
    };

    /**
     * Maintains the state of a store cell.
     */
    template <Cell C>
    class store_cell_state : public compute_cell_state<store_cell_compute_state<C>> {
        /** Shorthand for the value type of C */
        typedef C::value_type value_type;

        /** Shorthand for parent class */
        typedef compute_cell_state<store_cell_compute_state<C>> parent;

    public:
        /**
         * Create a store cell state.
         *
         * @param k    Key identifying the cell
         * @param cell The argument cell
         */
        store_cell_state(key_ref k, C arg) :
            parent(k, arg) {}

    protected:

        void init() override {
            parent::init();

            this->compute.arg.add_observer(this->observer_ptr());

            try {
                // Compute the initial value
                this->value();
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }
        }

        void pause() override {
            this->compute.arg.remove_observer(this->observer_ptr());
            parent::pause();
        }
    };

    /**
     * A cell that stores the value of another cell in memory.
     *
     * This is useful to cache the computed value of a lightweight
     * computed cell, which would otherwise be recomputed every time
     * it is accessed.
     */
    template <Cell C>
    class store_cell : public stateful_cell<store_cell_state<C>> {
        /**
         * Shorthand for parent class
         */
        typedef stateful_cell<store_cell_state<C>> parent;

        /**
         * Cell key type
         */
        typedef store_cell_key<key_ref> key_type;

    public:
        typedef C::value_type value_type;

        /**
         * Create a store cell.
         *
         * The value of this cell is the value of the argument cell @a
         * cell.
         *
         * @param cell The argument cell.
         */
        store_cell(C cell) :
            parent(key_ref::create<key_type>(cell.key()), cell) {}

        value_type value() const {
            return this->state->value();
        }

        value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }
    };

    /**
     * Create a cell which caches the value of @a cell in memory.
     *
     * This is useful if @a cell is a lightweight computed cell. The
     * returned cell caches its value in memory. Accessing the value
     * through the returned cell guarantees that the value of @a cell
     * is only recomputed, when it will actually change instead of
     * computing the value every time it is accessed.
     *
     * @param arg The argument cell
     *
     * @return A cell which has the same value as @a arg, but caches
     * it in memory when it hasn't changed.
     */
    template <Cell C>
    auto store(const C &arg) {
        return store_cell<C>(arg);
    }

}  // live_cells

#endif /* LIVE_CELLS_STORE_CELL_HPP */
