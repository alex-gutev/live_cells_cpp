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

#ifndef LIVE_CELLS_CELL_HPP
#define LIVE_CELLS_CELL_HPP

#include <memory>

#include "keys.hpp"
#include "observable.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * Base cell interface.
     *
     * A cell is an observable with a value. This interface provides
     * an accessor for retrieving the cell's value.
     */
    template <typename T>
    class cell : public observable {
    public:
        using observable::observable;

        /**
         * The type of value held by this cell type.
         */
        typedef T value_type;

        /**
         * Get the value of the cell.
         *
         * @return The value
         */
        virtual T value() const = 0;
    };

    /**
     * Reference an argument cell.
     *
     * When called in a context where the referenced argument cells
     * are tracked dynamically, this function adds @a cell to the set
     * of referenced argument cells.
     *
     * @return cell The cell to reference
     *
     * @return The value of the cell.
     */
    template <typename T>
    auto use(T cell) {
        argument_tracker::global().track_argument(
            observable_ref(cell)
        );

        return cell.value();
    }

}  // live_cells

#endif /* LIVE_CELLS_CELL_HPP */
