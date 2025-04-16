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

#ifndef LIVE_CELLS_MUTABLE_DEPENDENT_CELL_HPP
#define LIVE_CELLS_MUTABLE_DEPENDENT_CELL_HPP

#include <memory>
#include <functional>
#include <unordered_set>

#include "observable.hpp"
#include "mutable_cell.hpp"
#include "exceptions.hpp"
#include "observer_cell_state.hpp"

namespace live_cells {

    /**
     * \brief Maintains the state of a mutable computed cell.
     */
    template <typename T>
    class mutable_compute_cell_state :
        public mutable_cell_state<T>,
        public observer,
        public observer_cell_state {

        /** \brief Shorthand for parent */
        typedef mutable_cell_state<T> parent;

    public:

        /**
         * \brief Create the state for a mutable computed cell.
         *
         * \param key Key identifying the cell.
         *
         * \param arguments Set of argument cells referenced in value
         *   computation function.
         */
        mutable_compute_cell_state(key_ref key, const std::unordered_set<cell> &arguments) :
            parent(key),
            arguments(arguments) {}

        /**
         * \brief Get the value of the cell.
         *
         * \return The value of the cell.
         */
        T value() {
            if (stale) {
                computed = true;

                try {
                    this->silent_set(compute());
                }
                catch (const stop_compute_exception &) {
                    // Prevent value from being updated
                }

                stale = !this->is_active();
            }

            return parent::value();
        }

        /**
         * \brief Set the value of the cell.
         *
         * \param value The new value.
         */
        void value(T value) {
            reverse = true;
            this->notify_will_update();

            updating = false;
            stale = false;
            computed = false;

            this->silent_set(value);

            try {
                batch([&, this] () {
                    reverse_compute(value);
                });
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }

            if (parent::is_batch_update()) {
                this->add_to_batch();
            }
            else {
                this->notify_update();
            }

            reverse = false;
        }

        void will_update(const key_ref &k) override {
            if (!reverse) {
                handle_will_update([this] {
                    this->notify_will_update();
                });
            }
        }

        void update(const key_ref &k, bool changed) override {
            handle_update(changed, [this] (bool changed) {
                this->notify_update(changed);
            });
        }

        /**
         * \brief Add this state as an observer to the compute
         * function arguments.
         */
        void init() override {
            cell_state::init();
            init_observer_state();

            try {
                this->silent_set(this->value());
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }
        }

        /**
         * \brief Remove this state from the observers of the compute
         * function arguments.
         */
        void pause() override {
            cell_state::pause();
            pause_observer_state();

            for (auto arg : arguments) {
                arg.remove_observer(observer_ptr());
            }
        }

    protected:
        /**
         * \brief Set of argument cells referenced by the value
         * computation function.
         */
        std::unordered_set<cell> arguments;

        /**
         * \brief Is the value of the cell being set, and hence a
         * reverse computation being performed?
         */
        bool reverse = false;

        /**
         * \brief Is the cell's value the computed value (true) or a
         * value that was set (false).
         */
        bool computed = true;

        /**
         * \brief Compute the value of the cell as a function of its
         * argument cells.
         *
         * Subclasses should override this method with a method that
         * performs the computation logic for the cell.
         *
         * \return The computed value
         */
        virtual T compute() = 0;

        /**
         * \brief Perform the reverse computation.
         *
         * This method is called when the value of the cell is set
         * explicitly.
         *
         * Subclasses should override this method with a method that
         * sets the values of the argument cells such that the value
         * that is returned by \p compute() is equal to \a value.
         *
         * \param value The assigned cell value.
         */
        virtual void reverse_compute(T value) = 0;

        /**
         * \brief Get an \p observer::ref for this, that can be passed
         * to \p add_observer() and \p remove_observer() of the \p
         * Cell concept.
         *
         * \return Shared pointer to this state
         */
        std::shared_ptr<observer> observer_ptr() {
            return std::dynamic_pointer_cast<observer>(this->shared_from_this());
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_DEPENDENT_CELL_HPP */
