#ifndef LIVE_CELLS_DEPENDENT_CELL_HPP
#define LIVE_CELLS_DEPENDENT_CELL_HPP

#include "cell.hpp"
#include "merged_observable.hpp"

namespace live_cells {

    /**
     * Provides an interface for a cell with a value that is dependent
     * on one or more argument cells.
     *
     * The observers of this cell are notified when the values of the
     * argument cells change.
     */
    template <typename T, typename... Ts>
    class dependent_cell : cell<T> {
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
        dependent_cell(Ts&&... args) : observable(args...) {}

        void add_observer(observer::ref o) {
            observable.add_observer(o);
        }

        void remove_observer(observer::ref o) {
            observable.remove_observer(o);
        }

    protected:
        merged_observable<Ts...> observable;
    };


}  // live_cells

#endif /* LIVE_CELLS_DEPENDENT_CELL_HPP */
