#ifndef LIVE_CELLS_DEPENDENT_CELL_HPP
#define LIVE_CELLS_DEPENDENT_CELL_HPP

#include <memory>

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
    class dependent_cell : public cell<T> {
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
        dependent_cell(Ts... args) : observable(args...) {}

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
            cell<T>(key),
            observable(args...) {}

        void add_observer(observer::ref o) override {
            observable.add_observer(o);
        }

        void remove_observer(observer::ref o) override {
            observable.remove_observer(o);
        }

    protected:
        merged_observable<Ts...> observable;
    };


}  // live_cells

#endif /* LIVE_CELLS_DEPENDENT_CELL_HPP */
