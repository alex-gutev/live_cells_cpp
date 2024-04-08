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

#ifndef LIVE_CELLS_DEPENDENT_CELL_HPP
#define LIVE_CELLS_DEPENDENT_CELL_HPP

#include <memory>

#include "observable.hpp"
#include "merged_observable.hpp"

namespace live_cells {

    /**
     * \brief Provides an interface for a cell with a value that is
     * dependent on one or more argument cells.
     *
     * The observers of this cell are notified when the values of the
     * argument cells change.
     *
     * \note This cell is stateless with its value computed whenever it is
     * accessed and its observers added directly to the argument
     * cells.
     *
     * \note This is not a complete class and hence does not satisfy
     * the \p Cell concept. Subclasses should a \p value() method that
     * computes the cells value.
     */
    template <Cell... Ts>
    class dependent_cell {
    public:

        /**
         * \brief Create a cell with a value dependent on the argument
         * cells in \a args.
         *
         * The observers of this cell are notified whenever the value
         * of one of \a args changes.
         *
         * \param args Argument cells
         */
        dependent_cell(Ts... args) :
            observable(args...) {}

        void add_observer(observer::ref o) const {
            observable.add_observer(o);
        }

        void remove_observer(observer::ref o) const {
            observable.remove_observer(o);
        }

    protected:
        merged_observable<Ts...> observable;
    };


}  // live_cells

#endif /* LIVE_CELLS_DEPENDENT_CELL_HPP */
