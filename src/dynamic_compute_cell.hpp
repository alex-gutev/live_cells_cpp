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

#ifndef LIVE_CELLS_DYNAMIC_COMPUTE_CELL_HPP
#define LIVE_CELLS_DYNAMIC_COMPUTE_CELL_HPP

#include <unordered_set>
#include <functional>
#include <utility>

#include "compute_state.hpp"
#include "stateful_cell.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * State for a computed cell which determines its argument cells
     * dynamically.
     */
    template <typename T>
    class dynamic_compute_cell_state : public compute_cell_state<T> {
    public:

        /**
         * Create a dynamic computed cell state, with a given value
         * computation function.
         *
         * @param key     Key identifying cell state
         * @param compute Value computation function.
         */
        template <typename F>
        dynamic_compute_cell_state(key_ref key, F&& compute) :
            compute_cell_state<T>(key),
            compute_(std::forward<F>(compute)) {}

    protected:
        T compute() override {
            auto t = argument_tracker::global().with_tracker([this] (auto cell) {
                if (!arguments.count(cell)) {
                    arguments.emplace(cell);

                    cell->add_observer(this->observer_ptr());
                }
            });

            return compute_();
        }

    private:
        /**
         * Set of argument cells referenced by value computation function.
         */
        std::unordered_set<observable_ref> arguments;

        /**
         * Value computation function.
         */
        std::function<T()> compute_;

        void init() override {
            compute_cell_state<T>::init();

            try {
                // Determine arguments and add observers
                this->value();
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }
        }

        void pause() override {
            compute_cell_state<T>::pause();

            for (auto arg : arguments) {
                arg->remove_observer(this->observer_ptr());
            }

            arguments.clear();
        }
    };

    /**
     * A computed cell which determines its argument cells at runtime.
     */
    template <typename T>
    class dynamic_compute_cell : public stateful_cell<T, dynamic_compute_cell_state<T>> {
        /**
         * Shorthand for parent class
         */
        typedef stateful_cell<T, dynamic_compute_cell_state<T>> parent;

    public:
        /**
         * Create a dynamic computed cell with a given value
         * computation function.
         *
         * @param compute Value computation function.
         */
        dynamic_compute_cell(std::function<T()> compute) :
            dynamic_compute_cell(key_ref::create<unique_key>(), compute) {}

        /**
         * Create a dynamic computed cell with a given value
         * computation function.
         *
         * @param key     Key identifying cell.
         * @param compute Value computation function.
         */
        dynamic_compute_cell(key_ref key, std::function<T()> compute) :
            parent(key, compute) {}

        T value() const override {
            return this->state->value();
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_DYNAMIC_COMPUTE_CELL_HPP */
