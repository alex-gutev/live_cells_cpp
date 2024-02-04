#ifndef LIVE_CELLS_MUTABLE_CELL_HPP
#define LIVE_CELLS_MUTABLE_CELL_HPP

#include "cell_state.hpp"
#include "stateful_cell.hpp"

namespace live_cells {

    /**
     * Maintains the state of a mutable cell.
     */
    template<typename T>
    class mutable_cell_state : public cell_state {
    public:
        using cell_state::cell_state;

        /**
         * Create a mutable cell state with a value initialized to @a value.
         *
         * @param key    Key identifying the state
         * @param value  Initial value
         */
        template <typename K>
        mutable_cell_state(std::shared_ptr<K> key, T value) :
            cell_state(key),
            value_(value) {
        }

        /**
         * Retrieve the cell's value.
         */
        T value() {
            return value_;
        }

        /**
         * Set the cell's value and notify its observers.
         */
        void value(T value) {
            notify_will_update();
            value_ = std::move(value);
            notify_update();
        }

    private:
        /** The cell's value */
        T value_;

    };

    /**
     * A stateful cell which can have its value set directly.
     */
    template<typename T>
    class mutable_cell : public stateful_cell<T, mutable_cell_state<T>> {
        /**
         * Shorthand for parent class
         */
        typedef stateful_cell<T, mutable_cell_state<T>> parent;

    public:
        /**
         * Create a mutable cell with a default initialized value.
         */
        mutable_cell() : parent(std::make_shared<unique_key>()) {}

        /**
         * Create a mutable cell with an initial value.
         *
         * @param value The initial value of the cell.
         */
        mutable_cell(T value) :
            mutable_cell(std::make_shared<unique_key>(), value) {}

        /**
         * Create a mutable cell with an initial value.
         *
         * If there is no state associated with key @a key, a new
         * state is created and the value of the cell is initialized
         * to @a value.
         *
         * If there is a state associated with @a key, @a value is
         * ignored.
         *
         * @param key   Key identifying cell
         * @param value Initial value
         */
        template <typename K>
        mutable_cell(std::shared_ptr<K> key, T value) :
            parent(key, value) {
        }

        T value() const override {
            return this->state->value();
        }

        /**
         * Set the value of the cell.
         *
         * @param value The new value
         */
        void value(T value) {
            this->state->value(value);
        }
    };

    /**
     * Create a new mutable cell with an initial value.
     *
     * @param value The initial value
     *
     * @return A mutable cell
     */
    template<typename T>
    mutable_cell<T> variable(T value) {
        return mutable_cell<T>(value);
    }

    /**
     * Create a new mutable cell with a key and an initial value.
     *
     * NOTE: @a value is ignored if @a key already identifies a
     * mutable cell state.
     *
     * @param key   Key identifying cell
     * @param value The initial value
     *
     * @return A mutable cell
     */
    template<typename K, typename T>
    mutable_cell<T> variable(std::shared_ptr<K> key, T value) {
        return mutable_cell<T>(key, value);
    }

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_CELL_HPP */
