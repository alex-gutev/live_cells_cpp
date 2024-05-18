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

#ifndef LIVE_CELLS_STORE_CELL_HPP
#define LIVE_CELLS_STORE_CELL_HPP

#include "keys.hpp"
#include "compute_state.hpp"
#include "stateful_cell.hpp"
#include "changes_only_state.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * \brief Key identifying a \p store_cell
     */
    template <typename T>
    struct store_cell_key : value_key<T> {
        using value_key<T>::value_key;
    };

    template <Cell C>
    class store_cell_state;

    /**
     * \brief Defines the computation function of a \p store_cell.
     */
    template <Cell C>
    class store_cell_compute_state {
        /**
         * \brief Argument cell
         */
        const C arg;

        friend class store_cell_state<C>;

    public:
        /**
         * Create the computation state of a \p store_cell.
         *
         * \param arg The argument cell.
         */
        store_cell_compute_state(C arg) :
            arg(arg) {}

        /**
         * \brief Compute the value of the argument cell.
         *
         * \param ref The \p store_cell state
         *
         * \return The computed value
         */
        C::value_type operator()(observer::ref) {
            return arg.value();
        }
    };

    /**
     * \brief Maintains the state of a \p store_cell.
     */
    template <Cell C>
    class store_cell_state : public compute_cell_state<store_cell_compute_state<C>> {
        /** Shorthand for parent class */
        typedef compute_cell_state<store_cell_compute_state<C>> parent;

    public:
        /**
         * \brief Create a \p store_cell state.
         *
         * \param k    Key identifying the cell
         * \param cell The argument cell
         */
        store_cell_state(key_ref k, C arg) :
            parent(k, arg) {}

    protected:
        /** Shorthand for the value type of C */
        typedef C::value_type value_type;

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
     * \brief A \p store_cell_state that only notifies the observers
     * of the cell if the new value of the cell is not equal to its
     * previous value.
     */
    template <Cell C>
    class store_changes_only_cell_state :
        public changes_only_cell_state<store_cell_state<C>> {
        typedef changes_only_cell_state<store_cell_state<C>> parent;

    public:
        using parent::parent;

    };

    /**
     * \brief A \p Cell that caches the value of another \p Cell.
     *
     * This cell reads the value of another cell and caches it until
     * it changes.
     *
     * This is useful to cache the computed value of a lightweight
     * computed cell, which would otherwise be recomputed every time
     * it is accessed.
     */
    template <Cell C, typename State = store_cell_state<C>>
    class store_cell : public stateful_cell<State> {
        /**
         * Shorthand for parent class
         */
        typedef stateful_cell<State> parent;

        /**
         * Cell key type
         */
        typedef store_cell_key<key_ref> key_type;

    public:
        /**
         * \brief The type of value held by this cell.
         */
        typedef C::value_type value_type;

        /**
         * \brief Create a store cell that caches the value of \a
         * cell.
         *
         * \param cell The argument cell.
         */
        store_cell(C cell) :
            parent(key_ref::create<key_type>(cell.key()), cell) {}

        /**
         * \brief Get the value of the cell.
         *
         * \return The value of the cell.
         */
        value_type value() const {
            return this->state->value();
        }

        /**
         * \brief Get the value of the cell and track it as a
         * dependency.
         *
         * \return The value of the cell.
         */
        value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }
    };

    /**
     * \brief Create a \p Cell that caches the value of another \p
     * Cell.
     *
     * The returned cell reads the value of \a arg and caches it until
     * it changes.
     *
     * This cell guarantes that the value of \a arg is only recomputed
     * when it when its dependencies change instead of being computed
     * every time it is accessed, provided its value is only accessed
     * through the returned cell.
     *
     * \param arg The argument cell
     *
     * \return A cell which has the same value as \a arg, but caches
     *    it in memory until it changes.
     */
    template <Cell C>
    auto store(const C &arg) {
        return store_cell<C>(arg);
    }

    /**
     * \brief Create a \p Cell that caches the value of another \p
     * Cell and only notifies its observers when its value has
     * actually changed.
     *
     * The returned cell reads the value of \a arg and caches it until
     * it changes.
     *
     * This cell guarantes that the value of \a arg is only recomputed
     * when it when its dependencies change instead of being computed
     * every time it is accessed, provided its value is only accessed
     * through the returned cell.
     *
     * \note The difference between this and the other overloads, is
     * that with this overload, the cell only notifies its observers
     * when its new value is not equal to its previous value.
     *
     * \param option Changes only cell option
     *
     * \param arg The argument cell
     *
     * \return A cell which has the same value as \a arg, but caches
     *    it in memory until it changes.
     */
    template <Cell C>
    auto store(changes_only_option option, const C &arg) {
        return store_cell<C, store_changes_only_cell_state<C>>(arg);
    }

    namespace ops {

        /**
         * \brief Operator for caching the value of another cell.
         *
         * \see live_cells::store
         *
         * \param cell The operand \p Cell
         *
         * \return A \p Cell that has the same value as \a cell but
         *    caches it in memory until it changes.
         */
        constexpr auto store = [] (const Cell auto &cell) {
            return live_cells::store(cell);
        };

        /**
         * \brief Operator for creating a cell that caches the value
         * of another cell and only notifies its observers when its
         * value has actually changed.
         *
         * \see live_cells::store
         *
         * \param cell The operand \p Cell
         *
         * \return A \p Cell that has the same value as \a cell but
         *    caches it in memory until it changes.
         */
        constexpr auto cache = [] (const Cell auto & cell) {
            return live_cells::store(changes_only, cell);
        };

    }  // ops

}  // live_cells

#endif /* LIVE_CELLS_STORE_CELL_HPP */
