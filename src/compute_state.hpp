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

#ifndef LIVE_CELLS_COMPUTE_STATE_HPP
#define LIVE_CELLS_COMPUTE_STATE_HPP

#include <memory>
#include <utility>
#include <concepts>
#include <cassert>

#include "observable.hpp"
#include "cell_state.hpp"
#include "exceptions.hpp"
#include "observer_cell_state.hpp"

namespace live_cells {

    /**
     * \brief Defines a type that computes a value when the call
     * operator \p operator() is invoked.
     *
     * The call operator must accept a single \p observer::ref
     * argument.
     */
    template <typename T>
    concept Computable = std::invocable<T,observer::ref>;

    /**
     * \brief Cell state for a cell which computes a value as a function of
     * one or more argument cells.
     *
     * The actual computation is defined by the \p Computable \a C.
     */
    template <Computable C>
    class compute_cell_state : public cell_state, public observer, public observer_cell_state<> {
    public:
        /**
         * \brief Shorthand for computed value type
         */
        typedef std::invoke_result_t<C,observer::ref> value_type;

        /**
         * \brief Create a computed cell state.
         *
         * \param k Key identifying the cell
         *
         * \param args Arguments forwarded to the constructor of \a C.
         */
        template <typename... Args>
        requires std::constructible_from<C,Args...>
        compute_cell_state(key_ref k, Args&&... args) :
            cell_state(k),
            compute(std::forward<Args>(args)...) {}

        /**
         * \brief Retrieve the latest cached value.
         *
         * The value is computed using \p C::operator(), which is passed
         * a \p shared_ptr, of type \p observer, pointing to \p this.
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
         * \brief Compute value function.
         */
        C compute;

        /**
         * \brief Get an \p observer::ref for this, that can be passed to
         * \p add_observer and \p remove_observer of \p Cell.
         */
        std::shared_ptr<observer> observer_ptr() {
            return std::dynamic_pointer_cast<observer>(this->shared_from_this());
        }

        void will_update(const key_ref &k) override {
            handle_will_update([this] {
                notify_will_update();
            });
        }

        void update(const key_ref &k, bool changed) override {
            handle_update(changed, [this] (bool changed) {
                notify_update(changed);
            });
        }

        void init() override {
            cell_state::init();
            init_observer_state();
        }

        void pause() override {
            cell_state::pause();
            pause_observer_state();
        }

    private:
        /**
         * \brief The cached value.
         */
        value_type value_;
    };

}  // live_cells

#endif /* LIVE_CELLS_COMPUTE_STATE_HPP */
