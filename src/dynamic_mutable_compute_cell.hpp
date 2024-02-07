#ifndef LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP
#define LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP

#include <functional>
#include <memory>

#include "keys.hpp"
#include "mutable_compute_cell_state.hpp"

namespace live_cells {

    /**
     * Maintains the state for a mutable computed cell with a
     * dynamically determined argument list.
     */
    template<typename T>
    class dynamic_mutable_compute_cell_state : public mutable_compute_cell_state<T> {

        /** Shorthand for parent class */
        typedef mutable_compute_cell_state<T> parent;

    public:

        /**
         * Create the state for a dynamic mutable computed cell.
         *
         * @param key       Key identifying cell
         * @param compute   Value computation function
         * @param reverse   Reverse computation function
         */
        template <typename K, typename C, typename R>
        dynamic_mutable_compute_cell_state(std::shared_ptr<K> key, C&& compute, R&& reverse) :
            parent(key, {}),
            compute_fn(std::forward<C>(compute)),
            reverse_fn(std::forward<R>(reverse)) {}

    protected:
        T compute() override {
            auto t = argument_tracker::global().with_tracker([this] (auto cell) {
                if (!this->arguments.count(cell)) {
                    this->arguments.emplace(cell);

                    cell->add_observer(this->observer_ptr());
                }
            });

            return compute_fn();
        }

        void reverse_compute(T value) override {
            reverse_fn(value);
        }


        void init() override {
            parent::init();

            try {
                // Determine arguments and add observers
                compute();
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }
        }

    private:
        /** Value computation function */
        std::function<T()> compute_fn;

        /** Reverse computation function */
        std::function<void(T)> reverse_fn;

    };

    /**
     * A mutable computed cell with the argument cells determined
     * dynamically
     */
    template <typename T>
    class dynamic_mutable_compute_cell :
        public stateful_cell<T, dynamic_mutable_compute_cell_state<T>> {

        /** Shorthand for parent class */
        typedef stateful_cell<T, dynamic_mutable_compute_cell_state<T>> parent;

    public:

        /**
         * Create a dynamic mutable computed cell.
         *
         * @param k Key identifying cell
         *
         * @param compute Compute value function.
         *
         *   This function should compute the cell's value as a
         *   function of the cells in @a args.
         *
         * @param reverse Reverse computation function.
         *
         *   This function should set the values of the cells in @a
         *   args, such that @a compute returns the same value as the
         *   value that was assigned to the cell, which is passed to
         *   this function.
         */
        template <typename K, typename C, typename R>
        dynamic_mutable_compute_cell(std::shared_ptr<K> k, C&& compute, R&& reverse) :
            parent(k, std::forward<C>(compute), std::forward<R>(reverse)) {}

        template <typename C, typename R>
        dynamic_mutable_compute_cell(C&& compute, R&& reverse) :
            parent(std::make_shared<unique_key>(), std::forward<C>(compute), std::forward<R>(reverse)) {}

        T value() const override {
            return this->state->value();
        }

        /**
         * Set the value of the cell.
         *
         * This method triggers the reverse computation function of
         * the cell.
         *
         * NOTE: This method is marked const to allow the value of the
         * cell to be set when it is copy-captured by a lambda.
         */
        void value(T value) const {
            this->state->value(value);
        }

    };

}  // live_cells

#endif /* LIVE_CELLS_DYNAMIC_MUTABLE_COMPUTE_CELL_HPP */
