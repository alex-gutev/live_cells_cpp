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

#ifndef LIVE_CELLS_DEPENDENT_CELL_HPP
#define LIVE_CELLS_DEPENDENT_CELL_HPP

#include <memory>

#include "observable.hpp"
#include "merged_observable.hpp"

namespace live_cells {

    /**
     * Provides an interface for a cell with a value that is dependent
     * on one or more argument cells.
     *
     * The observers of this cell are notified when the values of the
     * argument cells change.
     */
    template <typename T, Cell... Ts>
    class dependent_cell {
    public:

        /**
         * Create a cell with a value dependent on the argument cells
         * @a args.
         *
         * The observers of this cell are notified whenever the values
         * of one of @a args changes.
         *
         * @param args Argument cells
         */
        dependent_cell(Ts... args) :
            key_(key_ref::create<unique_key>()),
            observable(args...) {}

        /**
         * Create a cell with a value dependent on the argument cells
         * @a args.
         *
         * The observers of this cell are notified whenever the values
         * of one of @a args changes.
         *
         * @param args Argument cells
         * @param key Key identifying cell
         */
        dependent_cell(key_ref key, Ts... args) :
            key_(key),
            observable(args...) {}

        void add_observer(observer::ref o) const {
            observable.add_observer(o);
        }

        void remove_observer(observer::ref o) const {
            observable.remove_observer(o);
        }

        key_ref key() const {
            return key_;
        }

    protected:
        const key_ref key_;
        merged_observable<Ts...> observable;
    };


}  // live_cells

#endif /* LIVE_CELLS_DEPENDENT_CELL_HPP */
