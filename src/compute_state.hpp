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

#include "observable.hpp"
#include "cell_state.hpp"
#include "exceptions.hpp"

namespace live_cells {

    /**
     * Cell state for a cell which computes a value, of type @a T,
     * that is a function of one or more argument cells.
     */
    template <typename T>
    class compute_cell_state : public cell_state, public observer {
    public:
        using cell_state::cell_state;

        /**
         * Retrieve the latest value
         */
        T value() {
            if (stale) {
                try {
                    value_ = compute();
                }
                catch (const stop_compute_exception &) {
                    // Prevent value from being updated
                }

                stale = !is_active();
            }

            return value_;
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
         * Compute the value of the cell.
         *
         * Subclasses should override this method, and place the value
         * computation function here.
         */
        virtual T compute() = 0;

        /**
         * Get an observer::ref for this, that can be passed to
         * observable::add_observer and observable::remove_observer.
         */
        std::shared_ptr<observer> observer_ptr() {
            return std::dynamic_pointer_cast<observer>(this->shared_from_this());
        }

    private:
        T value_;
    };

}  // live_cells

#endif /* LIVE_CELLS_COMPUTE_STATE_HPP */
