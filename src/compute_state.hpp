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

#ifndef LIVE_CELLS_COMPUTE_STATE_HPP
#define LIVE_CELLS_COMPUTE_STATE_HPP

#include <memory>
#include <utility>
#include <concepts>

#include "observable.hpp"
#include "cell_state.hpp"
#include "exceptions.hpp"

namespace live_cells {

    /**
     * Defines a type that computes a value when the call operator
     * `operator()` is invoked.
     *
     * The call operator must accept a single `observer::ref`
     * argument.
     */
    template <typename T>
    concept Computable = std::invocable<T,observer::ref>;

    /**
     * Cell state for a cell which computes a value as a function of
     * one or more argument cells.
     *
     * The actual computation is defined by the Computable `C`.
     */
    template <Computable C>
    class compute_cell_state : public cell_state, public observer {
    public:
        /**
         * Shorthand for computed value type
         */
        typedef std::invoke_result_t<C,observer::ref> value_type;

        /**
         * Create a computed cell state.
         *
         * @param k Key identifying cell
         *
         * @param args Arguments forwarded to the constructor of `C`.
         */
        template <typename... Args>
        requires std::constructible_from<C,Args...>
        compute_cell_state(key_ref k, Args&&... args) :
            cell_state(k),
            compute(std::forward<Args>(args)...) {}

        /**
         * Retrieve the latest value
         *
         * The value is computed using C::operator(), which is passed
         * a shared_ptr, of type `observer`, to `this`.
         */
        value_type value() {
            if (stale) {
                try {
                    value_ = compute(observer_ptr());
                }
                catch (const stop_compute_exception &) {
                    // Prevent value from being updated
                }

                stale = !is_active();
            }

            return value_;
        }

    protected:
        /**
         * Does the current have to be recomputed?
         */
        bool stale = true;

        /**
         * Are the argument cells in the process of updating their
         * values.
         */
        bool updating = false;

        /**
         * Compute value function.
         */
        C compute;

        /**
         * Get an observer::ref for this, that can be passed to
         * observable::add_observer and observable::remove_observer.
         */
        std::shared_ptr<observer> observer_ptr() {
            return std::dynamic_pointer_cast<observer>(this->shared_from_this());
        }

        void will_update(const key_ref &k) override {
            if (!updating) {
                updating = true;

                notify_will_update();
                stale = true;
            }
        }

        void update(const key_ref &k) override {
            if (updating) {
                notify_update();
                updating = false;
            }
        }

        void init() override {
            cell_state::init();
            stale = true;
        }

        void pause() override {
            cell_state::pause();
            stale = true;
        }

    private:
        value_type value_;
    };

}  // live_cells

#endif /* LIVE_CELLS_COMPUTE_STATE_HPP */
