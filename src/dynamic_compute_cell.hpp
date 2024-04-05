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
#include <concepts>
#include <type_traits>

#include "compute_state.hpp"
#include "stateful_cell.hpp"
#include "tracker.hpp"

namespace live_cells {

    template <std::invocable F>
    class dynamic_compute_cell_state;

    /**
     * \brief Defines the computation function of the state of a
     * \p dynamic_compute_cell.
     */
    template <std::invocable F>
    class dynamic_compute_state {
        /**
         * \brief Value computation function.
         */
        const F compute;

        /**
         * \brief Set of argument cells referenced by value computation function.
         */
        std::unordered_set<cell> arguments;

        friend class dynamic_compute_cell_state<F>;

    public:
        /**
         * Create the computation state for a \p dynamic_compute_cell.
         *
         * The function \a compute is called with no arguments, to
         * compute the value of the cell. It should reference its
         * argument cells using the function call operator so that
         * they are registered as dependencies of the computed cell.
         *
         * \param compute Function of no arguments called to compute
         *    the value of the cell.
         */
        dynamic_compute_state(F compute) :
            compute(compute) {}

        /**
         * \brief Compute the value of \p dynamic_compute_cell.
         *
         * \param state The computed cell state.
         */
        auto operator()(observer::ref state) {
            auto t = argument_tracker::global().with_tracker([this, state] (auto cell) {
                if (!arguments.count(cell)) {
                    arguments.emplace(cell);

                    cell.add_observer(state);
                }
            });

            return compute();
        }
    };

    /**
     * \brief Maintains the state of a \p dynamic_compute_cell.
     */
    template <std::invocable F>
    class dynamic_compute_cell_state : public compute_cell_state<dynamic_compute_state<F>> {
        typedef compute_cell_state<dynamic_compute_state<F>> parent;

    public:
        /**
         * \brief Create a dynamic computed cell state, with a given
         * value computation function.
         *
         * \param key     Key identifying cell state
         * \param compute Value computation function.
         */
        dynamic_compute_cell_state(key_ref key, F compute) :
            parent(key, compute) {}

    protected:
        void init() override {
            parent::init();

            try {
                // Determine arguments and add observers
                this->value();
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }
        }

        void pause() override {
            parent::pause();

            for (auto arg : this->compute.arguments) {
                arg.remove_observer(this->observer_ptr());
            }

            this->compute.arguments.clear();
        }
    };

    /**
     * \brief A computed cell which determines its argument cells at runtime.
     *
     * This is a stateful cell that caches its value when computed and
     * maintains its own observer set.
     */
    template <std::invocable F>
    class dynamic_compute_cell : public stateful_cell<dynamic_compute_cell_state<F>> {
        /**
         * \brief Shorthand for parent class
         */
        typedef stateful_cell<dynamic_compute_cell_state<F>> parent;

    public:
        /**
         * \brief Shorthand for the type of value held by this cell.
         */
        typedef std::invoke_result_t<F> value_type;

        /**
         * \brief Create a dynamic computed cell with a given value
         * computation function.
         *
         * The function \a compute is called with no arguments to
         * compute the value of the cell when necessary. It should
         * reference its argument cells using the function call
         * operator so that they are registered as dependencies of
         * this cell.
         *
         * \param compute Value computation function.
         */
        dynamic_compute_cell(F compute) :
            dynamic_compute_cell(key_ref::create<unique_key>(), compute) {}

        /**
         * \brief Create a dynamic computed cell with a given value
         * computation function.
         *
         * The function \a compute is called with no arguments to
         * compute the value of the cell when necessary. It should
         * reference its argument cells using the function call
         * operator so that they are registered as dependencies of
         * this cell.
         *
         * \param key     Key identifying cell.
         * \param compute Value computation function.
         */
        dynamic_compute_cell(key_ref key, F compute) :
            parent(key, compute) {}

        value_type value() const {
            return this->state->value();
        }

        value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_DYNAMIC_COMPUTE_CELL_HPP */
