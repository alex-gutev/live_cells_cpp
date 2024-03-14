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

#ifndef LIVE_CELLS_STATEFUL_CELL_HPP
#define LIVE_CELLS_STATEFUL_CELL_HPP

#include <memory>

#include "observable.hpp"
#include "cell_state.hpp"

namespace live_cells {

    /**
     * Base class for a cell with a state.
     *
     * The state is associated with the cell by its key. The cell
     * holds a shared_pointer to the state. This allows cell objects
     * to be freely copied, and even recreated using the same key,
     * while still pointing to the same shared state.
     *
     * @a S is the class which will be used to hold the cell's state.
     *
     * NOTE: This class is not meant to be used polymorphically.
     */
    template <typename S>
    class stateful_cell {
    public:
        /**
         * Create a stateful cell and associate it with a state.
         *
         * If there is a state associated with key @a k, it is
         * associated with the constructed cell. Otherwise:
         *
         * 1. A new state is created by calling the constructor of @a
         *    S, passing the key @a k followed by @a args.
         *
         * 2. The state is associated with key @a k.
         *
         * 3. The state is associated with the constructed cell.
         *
         * @param k The key identifying the cell's state
         *
         * @param args Additional arguments to pass to the state
         *   constructor.
         */
        template <typename... Args>
        stateful_cell(key_ref k, Args... args) :
            key_(k),
            state(state_manager::global().get<S>(k, args...)) {
        }

        key_ref key() const {
            return key_;
        }

        void add_observer(observer::ref o) const {
            state->add_observer(o);
        }

        void remove_observer(observer::ref o) const {
            state->remove_observer(o);
        }

    protected:
        /**
         * Key identifying the cell
         */
        const key_ref key_;

        /**
         * Reference to the cell's state.
         */
        std::shared_ptr<S> state;
    };

}  // live_cells

#endif /* LIVE_CELLS_STATEFUL_CELL_HPP */
