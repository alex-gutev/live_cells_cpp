#ifndef LIVE_CELLS_DYNAMIC_COMPUTE_CELL_HPP
#define LIVE_CELLS_DYNAMIC_COMPUTE_CELL_HPP

#include <unordered_set>
#include <functional>
#include <utility>

#include "compute_state.hpp"
#include "stateful_cell.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * State for a computed cell which determines its argument cells
     * dynamically.
     */
    template <typename T>
    class dynamic_compute_cell_state : public compute_cell_state<T> {
    public:

        /**
         * Create a dynamic computed cell state, with a given value
         * computation function.
         *
         * @param key     Key identifying cell state
         * @param compute Value computation function.
         */
        template <typename K, typename F>
        dynamic_compute_cell_state(std::shared_ptr<K> key, F&& compute) :
            compute_cell_state<T>(key),
            compute_(std::function<T()>(std::forward<F>(compute))) {}

    protected:
        T compute() override {
            auto t = argument_tracker::global().with_tracker([this] (auto cell) {
                if (!arguments.count(cell)) {
                    arguments.emplace(cell);

                    cell->add_observer(this->observer_ptr());
                }
            });

            return compute_();
        }

    private:
        /**
         * Set of argument cells referenced by value computation function.
         */
        std::unordered_set<observable_ref> arguments;

        /**
         * Value computation function.
         */
        std::function<T()> compute_;

        void init() override {
            compute_cell_state<T>::init();

            // Determine arguments and add observers
            compute();
        }

        void pause() override {
            compute_cell_state<T>::pause();

            for (auto arg : arguments) {
                arg->remove_observer(this->observer_ptr());
            }

            arguments.clear();
        }
    };

    /**
     * A computed cell which determines its argument cells at runtime.
     */
    template <typename T>
    class dynamic_compute_cell : public stateful_cell<T, dynamic_compute_cell_state<T>> {
        /**
         * Shorthand for parent class
         */
        typedef stateful_cell<T, dynamic_compute_cell_state<T>> parent;

    public:
        /**
         * Create a dynamic computed cell with a given value
         * computation function.
         *
         * @param compute Value computation function.
         */
        dynamic_compute_cell(std::function<T()> compute) :
            dynamic_compute_cell(std::make_shared<unique_key>(), compute) {}

        /**
         * Create a dynamic computed cell with a given value
         * computation function.
         *
         * @param key     Key identifying cell.
         * @param compute Value computation function.
         */
        template <typename K>
        dynamic_compute_cell(std::shared_ptr<K> key, std::function<T()> compute) :
            parent(key, compute) {}

        T value() const override {
            return this->state->value();
        }
    };

    /**
     * Create a cell with a value that is a function of one or more
     * argument cells.
     *
     * The argument cells, referenced by @a compute, are determined
     * dynamically. For this to work, the values of the argument cells
     * must be referenced using live_cells::use() within @a compute().
     *
     * @param compute Cell value function
     *
     * @return The cell
     */
    template <typename F>
    auto compute(F&& compute) -> dynamic_compute_cell<decltype(compute())> {
        return dynamic_compute_cell<decltype(compute())>(std::forward<F>(compute));
    }

}  // live_cells

#endif /* LIVE_CELLS_DYNAMIC_COMPUTE_CELL_HPP */
