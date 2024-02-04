#ifndef LIVE_CELLS_STATEFUL_CELL_HPP
#define LIVE_CELLS_STATEFUL_CELL_HPP

#include <memory>

#include "cell.hpp"
#include "cell_state.hpp"

namespace live_cells {

    /**
     * Base class for a cell with a state.
     *
     * The state is associated with the cell by its key. The cell
     * holds a shared_pointer to the state. This allows cell objects
     * to be freely copied, and even recreated using the same key,
     * while still pointing to the same shared state.
     *
     * @a S is the class which will be used to hold the cell's state.
     */
    template <typename T, typename S>
    class stateful_cell : public cell<T> {
    public:
        /**
         * Create a stateful cell and associate it with a state.
         *
         * If there is a state associated with key @a k, it is
         * associated with the constructed cell. Otherwise:
         *
         * 1. A new state is created by calling the constructor of @a
         *    S, passing the key @a k followed by @a args.
         *
         * 2. The state is associated with key @a k.
         *
         * 3. The state is associated with the constructed cell.
         *
         * @param k The key identifying the cell's state
         *
         * @param args Additional arguments to pass to the state
         *   constructor.
         */
        template <typename K, typename... Args>
        stateful_cell(std::shared_ptr<K> k, Args... args) :
            cell<T>(k),
            state(state_manager::global().get<S>(k, args...)) {
        }

        void add_observer(observer::ref o) override {
            state->add_observer(o);
        }

        void remove_observer(observer::ref o) override {
            state->remove_observer(o);
        }

    protected:
        /**
         * Reference to the cell's state.
         */
        std::shared_ptr<S> state;
    };

}  // live_cells

#endif /* LIVE_CELLS_STATEFUL_CELL_HPP */
