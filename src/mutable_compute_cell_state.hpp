#ifndef LIVE_CELLS_MUTABLE_DEPENDENT_CELL_HPP
#define LIVE_CELLS_MUTABLE_DEPENDENT_CELL_HPP

#include <memory>
#include <functional>
#include <unordered_set>

#include "observable.hpp"
#include "mutable_cell.hpp"

namespace live_cells {

    /**
     * Maintains the state of a mutable computed cell.
     */
    template <typename T>
    class mutable_compute_cell_state :
        public mutable_cell_state<T>,
        public observer {

        /** Shorthand for parent */
        typedef mutable_cell_state<T> parent;

    public:

        /**
         * Create the state for a mutable computed cell.
         *
         * @param key Key identifying the cell.
         *
         * @param arguments Set of argument observables to value
         *   computation function.
         */
        mutable_compute_cell_state(key_ref key, const std::unordered_set<observable_ref> &arguments) :
            parent(key),
            arguments(arguments) {}

        /**
         * Retrieve the latest value
         */
        T value() {
            if (stale) {
                computed = true;

                this->silent_set(compute());
                stale = !this->is_active();
            }

            return parent::value();
        }

        /**
         * Set the cell's value.
         *
         * @param value The value
         */
        void value(T value) {
            reverse = true;
            this->notify_will_update();

            updating = false;
            stale = false;
            computed = false;

            this->silent_set(value);

            try {
                batch([=] () {
                    reverse_compute(value);
                });
            }
            catch (...) {
                // Prevent exception from being propagated to caller
            }

            if (parent::is_batch_update()) {
                this->add_to_batch();
            }
            else {
                this->notify_update();
            }

            reverse = false;
        }

        void will_update(const key_ref &k) override {
            if (!updating) {
                updating = true;

                this->notify_will_update();
                stale = true;
            }
        }

        void update(const key_ref &k) override {
            if (updating) {
                this->notify_update();
                updating = false;
            }
        }

        /**
         * Add this state as an observer to the arguments of the
         * compute function.
         */
        void init() override {
            cell_state::init();
            stale = true;

            for (auto arg : arguments) {
                arg->add_observer(observer_ptr());
            }
        }

        /**
         * Remove this state from the observer to the arguments of the
         * compute function.
         */
        void pause() override {
            cell_state::pause();
            stale = true;

            for (auto arg : arguments) {
                arg->remove_observer(observer_ptr());
            }
        }

    protected:
        /**
         * Set of argument cells referenced by the value computation
         * function.
         */
        std::unordered_set<observable_ref> arguments;

        /**
         * Does the current have to be recomputed?
         */
        bool stale = true;

        /**
         * Are the argument cells in the process of updating their
         * values.
         */
        bool updating = false;

        /**
         * Is the value of the cell being set, and hence a reverse
         * computation being performed?
         */
        bool reverse = false;

        /**
         * Is the cell's value the computed value (true) or a value that was
         * set (false).
         */
        bool computed = true;

        /**
         * Compute the value of the cell as a function of its argument
         * cells.
         *
         * Subclasses should override this method with a method that
         * performs the computation logic for the cell.
         *
         * @return The computed value
         */
        virtual T compute() = 0;

        /**
         * Perform the reverse computation.
         *
         * This method is called when the value of the cell is set
         * explicitly.
         *
         * Subclasses should override this method with a method that
         * sets the values of the argument cells such that the value
         * that is returned by compute() is equal to @a value.
         *
         * @param value The assigned cell value.
         */
        virtual void reverse_compute(T value) = 0;

        /**
         * Get an observer::ref for this, that can be passed to
         * observable::add_observer and observable::remove_observer.
         */
        std::shared_ptr<observer> observer_ptr() {
            return std::dynamic_pointer_cast<observer>(this->shared_from_this());
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_DEPENDENT_CELL_HPP */
