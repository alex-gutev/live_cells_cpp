#ifndef LIVE_CELLS_COMPUTE_STATE_HPP
#define LIVE_CELLS_COMPUTE_STATE_HPP

#include <memory>

#include "observable.hpp"
#include "cell_state.hpp"

namespace live_cells {

    /**
     * Cell state for a cell which computes a value that is a function
     * of one or more argument cells.
     */
    template <typename T>
    class compute_cell_state : public cell_state, public observer, public std::enable_shared_from_this<compute_cell_state<T>> {
    public:
        using cell_state::cell_state;

        /**
         * Retrieve the latest value
         */
        T value() {
            if (stale) {
                value_ = compute();
                stale = !is_active();
            }

            return value_;
        }

        void will_update(const key::ref &k) override {
            if (!updating) {
                updating = true;

                notify_will_update();
                stale = true;
            }
        }

        void update(const key::ref &k) override {
            if (updating) {
                notify_update();
                updating = false;
            }
        }

        void init() override {
            cell_state::init();
            stale = true;
        }

        void pause() override {
            cell_state::pause();
            stale = true;
        }

    protected:
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
         * Compute the value of the cell.
         *
         * Subclasses should override this method, and place the value
         * computation function here.
         */
        virtual T compute() = 0;

        /**
         * Get an observer::ref for this, that can be passed to
         * observable::add_observer and observable::remove_observer.
         */
        std::shared_ptr<observer> observer_ptr() {
            return std::static_pointer_cast<observer>(this->shared_from_this());
        }

    private:
        T value_;
    };

}  // live_cells

#endif /* LIVE_CELLS_COMPUTE_STATE_HPP */
