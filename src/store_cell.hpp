#ifndef LIVE_CELLS_STORE_CELL_HPP
#define LIVE_CELLS_STORE_CELL_HPP

#include "keys.hpp"
#include "compute_state.hpp"
#include "stateful_cell.hpp"

namespace live_cells {

    /**
     * Key identifying a store_cell
     */
    template <typename T>
    struct store_cell_key : value_key<T> {
        using value_key<T>::value_key;
    };

    /**
     * Maintains the state of a store cell.
     */
    template <typename T>
    class store_cell_state : public compute_cell_state<T> {
        /** Shorthand for parent class */
        typedef compute_cell_state<T> parent;

    public:
        /**
         * Create a store cell state.
         *
         * @param k    Key identifying the cell
         * @param cell The argument cell
         */
        store_cell_state(key_ref k, observable_ref cell) :
            parent(k),
            cell(cell) {}

    protected:

        void init() override {
            parent::init();

            cell->add_observer(this->observer_ptr());

            try {
                // Compute the initial value
                this->value();
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }
        }

        void pause() override {
            cell->remove_observer(this->observer_ptr());
            parent::pause();
        }

        T compute() override {
            return cell.value<T>();
        }

    private:
        /**
         * Store cell argument cell
         */
        observable_ref cell;

    };

    /**
     * A cell that stores the value of another cell in memory.
     *
     * This is useful to cache the computed value of a lightweight
     * computed cell, which would otherwise be recomputed every time
     * it is accessed.
     */
    template <typename T>
    class store_cell : public stateful_cell<T, store_cell_state<T>> {
        /**
         * Shorthand for parent class
         */
        typedef stateful_cell<T, store_cell_state<T>> parent;

        /**
         * Cell key type
         */
        typedef store_cell_key<observable_ref> key_type;

    public:
        /**
         * Create a store cell.
         *
         * The value of this cell is the value of the argument cell @a
         * cell.
         *
         * @param cell The argument cell.
         */
        store_cell(observable_ref cell) :
            parent(key_ref::create<key_type>(cell), cell) {}

        T value() const override {
            return this->state->value();
        }
    };

    /**
     * Create a cell which caches the value of @a cell in memory.
     *
     * This is useful if @a cell is a lightweight computed cell. The
     * returned cell caches its value in memory. Accessing the value
     * through the returned cell guarantees that the value of @a cell
     * is only recomputed, when it will actually change instead of
     * computing the value every time it is accessed.
     *
     * @param cell The cell
     *
     * @return A cell which has the same value as @a cell, but caches
     * it in memory when it hasn't changed.
     */
    template <typename T>
    auto store(const T &cell) {
        return store_cell<typename T::value_type>(observable_ref(cell));
    }

}  // live_cells

#endif /* LIVE_CELLS_STORE_CELL_HPP */
