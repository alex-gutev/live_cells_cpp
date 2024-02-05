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
    auto use(T cell) -> decltype(cell.value()) {
        argument_tracker::global().track_argument(
            observable_ref(cell)
        );

        return cell.value();
    }

}  // live_cells

#endif /* LIVE_CELLS_CELL_HPP */
