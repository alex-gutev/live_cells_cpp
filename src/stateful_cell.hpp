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

#ifndef LIVE_CELLS_STATEFUL_CELL_HPP
#define LIVE_CELLS_STATEFUL_CELL_HPP

#include <memory>

#include "observable.hpp"
#include "cell_state.hpp"

namespace live_cells {

    /**
     * \brief Base class for a cell with a state.
     *
     * The state is associated with the cell by its key. The cell
     * holds a \p shared_ptr to the state. This allows cell objects
     * to be freely copied, and even recreated using the same key,
     * while still pointing to the same shared state.
     *
     * \a S is the class which will be used to hold the cell's state.
     *
     * \warning This class may be inherited but it is not meant to be
     * used polymorphically.
     */
    template <typename S>
    class stateful_cell {
    public:
        /**
         * \brief Create a stateful cell and associate it with a
         * state.
         *
         * If there is a state associated with key \a k, it is
         * associated with the constructed cell. Otherwise:
         *
         * 1. A new state is created by calling the constructor of \a
         *    S, passing the key \a k followed by \a args.
         *
         * 2. The state is associated with key \a k.
         *
         * 3. The state is associated with the constructed cell.
         *
         * \param k The key identifying the cell
         *
         * \param args Additional arguments to pass to the state
         *   constructor.
         */
        template <typename... Args>
        stateful_cell(key_ref k, Args... args) :
            key_(k),
            state(state_manager::global().get<S>(k, args...)) {
        }

        /**
         * \brief Get the key identifying the cell.
         *
         * \return The key identifying the cell
         */
        key_ref key() const {
            return key_;
        }

        /**
         * \brief Add an observer to the cell.
         *
         * \param o The observer to add.
         */
        void add_observer(observer::ref o) const {
            state->add_observer(o);
        }

        /**
         * \brief Remove an observer from the cell.
         *
         * \param o The observer to remove.
         */
        void remove_observer(observer::ref o) const {
            state->remove_observer(o);
        }

    protected:
        /**
         * \brief Key identifying the cell
         */
        const key_ref key_;

        /**
         * \brief Reference to the cell's state.
         */
        std::shared_ptr<S> state;
    };

}  // live_cells

#endif /* LIVE_CELLS_STATEFUL_CELL_HPP */
