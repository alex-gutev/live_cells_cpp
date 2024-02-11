#ifndef LIVE_CELLS_MUTABLE_CELL_HPP
#define LIVE_CELLS_MUTABLE_CELL_HPP

#include <memory>

#include "cell_state.hpp"
#include "stateful_cell.hpp"

namespace live_cells {

    /**
     * Defer changes to the values of mutable cells.
     *
     * When an instance of this object is in scope, observers of
     * mutable cells will not be notified immediately when the values
     * of the cells are set, but instead will be notified when this
     * object is destroyed.
     *
     * Creating a new instance when one already exists has no effect,
     * the observers will only be notified when the first instance is
     * destroyed.
     *
     * NOTE: Do not allocate instances of this object dynamically.
     */
    class batch_update {
    public:
        batch_update();
        ~batch_update();

        batch_update(const batch_update &) = delete;
        batch_update& operator=(const batch_update &) = delete;

    private:
        /** Is this instance responsible for notifying the observers */
        bool is_batching;

        /**
         * Is a batch update currently in effect?
         */
        static bool is_batch_update();

        /**
         * Add a mutable cell state to the list of batched updates.
         *
         * The observers of the cell associated with the state will be
         * notified with cell_state::notify_update() when the values
         * of all the cells in the batch have been set.
         */
        static void add_to_batch(std::shared_ptr<cell_state> state);

        template <typename T>
        friend class mutable_cell_state;
    };

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
        mutable_cell_state(key_ref key, T value) :
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
            if (value_ != value) {
                notify_will_update();
                value_ = std::move(value);

                if (!batch_update::is_batch_update()) {
                    notify_update();
                }
                else {
                    batch_update::add_to_batch(this->shared_from_this());
                }
            }
        }

    protected:
        /**
         * Set the cell's value without notifying observers.
         */
        void silent_set(T value) {
            value_ = std::move(value);
        }

        /**
         * Is a batch update of mutable cells currently in effect.
         */
        static bool is_batch_update() {
            return batch_update::is_batch_update();
        }

        /**
         * Add this state to the list of batched mutable cell value
         * updates.
         */
        void add_to_batch() {
            batch_update::add_to_batch(this->shared_from_this());
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
        mutable_cell() : parent(key_ref::create<unique_key>()) {}

        /**
         * Create a mutable cell with an initial value.
         *
         * @param value The initial value of the cell.
         */
        mutable_cell(T value) :
            mutable_cell(key_ref::create<unique_key>(), value) {}

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
        mutable_cell(key_ref key, T value) :
            parent(key, value) {
        }

        T value() const override {
            return this->state->value();
        }

        /**
         * Set the value of the cell.
         *
         * NOTE: This method is marked const to allow the value of the
         * cell to be set when it is copy-captured by a lambda.
         *
         * @param value The new value
         */
        void value(T value) const {
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
    template<typename T>
    mutable_cell<T> variable(key_ref key, T value) {
        return mutable_cell<T>(key, value);
    }

    /**
     * Batch changes to the values of mutable cells.
     *
     * The function @a fn is called with zero arguments. Observers of
     * mutable cells are only notified of changes to the cells' values
     * after @a fn returns.
     *
     * @param fn A function of zero arguments
     */
    template<typename F>
    void batch(F fn) {
        batch_update b;
        fn();
    }

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_CELL_HPP */
