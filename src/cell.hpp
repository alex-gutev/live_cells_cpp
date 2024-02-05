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

}  // live_cells

#endif /* LIVE_CELLS_CELL_HPP */
