#ifndef LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP
#define LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP

#include <functional>
#include <memory>

#include "keys.hpp"
#include "mutable_compute_cell_state.hpp"

namespace live_cells {

    /**
     * Maintains the state for a mutable computed cell with an
     * argument list determined at compile-time.
     */
    template <typename T>
    class static_mutable_compute_cell_state : public mutable_compute_cell_state<T> {
        /** Shorthand for parent class */
        typedef mutable_compute_cell_state<T> parent;

    public:

        /**
         * Create the state for a static mutable computed cell.
         *
         * @param key       Key identifying cell
         * @param compute   Value computation function
         * @param reverse   Reverse computation function
         * @param arguments Set of argument cells referenced in @a compute
         */
        template <typename C, typename R>
        static_mutable_compute_cell_state(key_ref key, C&& compute, R&& reverse, const std::unordered_set<observable_ref> &arguments) :
            parent(key, arguments),
            compute_fn(std::forward<C>(compute)),
            reverse_fn(std::forward<R>(reverse)) {}

    protected:
        T compute() override {
            return compute_fn();
        }

        void reverse_compute(T value) override {
            reverse_fn(value);
        }

    private:
        /** Value computation function */
        std::function<T()> compute_fn;

        /** Reverse computation function */
        std::function<void(T)> reverse_fn;

    };

    /**
     * A mutable computed cell with arguments determined at
     * compile-time.
     */
    template <typename T>
    class static_mutable_compute_cell :
        public stateful_cell<T, static_mutable_compute_cell_state<T>> {

        /** Shorthand for parent class */
        typedef stateful_cell<T, static_mutable_compute_cell_state<T>> parent;

    public:

        /**
         * Create a static mutable computed cell.
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
         *
         * @param args Arguments to @a compute
         */
        template <typename C, typename R, typename... Args>
        static_mutable_compute_cell(key_ref k, C&& compute, R&& reverse, Args&&... args) :
            parent(k, std::forward<C>(compute), std::forward<R>(reverse), {std::forward<Args>(args)...}) {}

        template <typename C, typename R, typename... Args>
        static_mutable_compute_cell(C&& compute, R&& reverse, Args&&... args) :
            parent(key_ref::create<unique_key>(), std::forward<C>(compute), std::forward<R>(reverse), std::unordered_set<observable_ref>({observable_ref(args)...})) {}

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

#endif /* LIVE_CELLS_STATIC_MUTABLE_COMPUTE_CELL_HPP */
