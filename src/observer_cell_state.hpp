#ifndef LIVE_CELLS_OBSERVER_CELL_STATE_HPP
#define LIVE_CELLS_OBSERVER_CELL_STATE_HPP

#include <cassert>
#include <concepts>

namespace live_cells {

    /**
     * \brief Provides functionality for observing a cell from a \p
     * cell_state.
     *
     * This class provides implementations of \p observer::will_update
     * and \p observer::update, namely \p handle_will_update and \p
     * handle_update, which determine when the cell's value should be
     * recomputed, and notify the observers of the cell.
     *
     * Subclasses should check the \p state member variable. If it is
     * true, the cell's value should be recomputed.
     */
    class observer_cell_state {
    protected:
        /**
         * \brief Does the value have to be recomputed?
         */
        bool stale = true;

        /**
         * \brief Are the argument cells in the process of updating their
         * values?
         */
        bool updating = false;

        /**
         * \brief Initialize the cell observation state.
         *
         * \attention This should be called from \p
         * cell_state::init().
         */
        void init_observer_state() {
            stale = true;
        }

        /**
         * \brief Pause the cell observation state.
         *
         * \attention This should be called from \p
         * cell_state::pause().
         */
        void pause_observer_state() {
            stale = true;
        }

        /**
         * \brief Handle a \p observer::will_update call.
         *
         * This method notifies the observers of this cell, by calling
         * \p notify_will_update.
         *
         * \attention This method should be called from \p
         * observer::will_update.
         *
         * \param notify_will_update Function that notifies the
         *    observers of the cell.
         */
        void handle_will_update(const std::invocable auto &notify_will_update) {
            if (!updating) {
                assert(changed_dependencies == 0 && "Number of changed dependencies == 0 at start of update cycle.");

                updating = true;
                has_changed = false;
                changed_dependencies = 0;

                notify_will_update();
            }

            changed_dependencies++;
        }

        /**
         * \brief Handle an \p observer::update call.
         *
         * This method notifies the observers of this cell, by calling
         * \p notify_update.
         *
         * \attention This method should be called from \p
         * observer::update.
         *
         * \param notify_will_update Function that notifies the
         *    observers of the cell.\n The argument passed to this
         *    cell is true if the cell's value may have changed or
         *    false if it has not changed.
         */
        void handle_update(bool changed, const std::invocable<bool> auto &notify_update) {
            if (updating) {
                assert(changed_dependencies > 0 && "Calls to update() are not more than calls to will_update().");

                has_changed = has_changed || changed;

                if (--changed_dependencies == 0) {
                    stale = stale || has_changed;

                    notify_update(has_changed);
                    updating = false;
                }
            }
        }

    private:

        /**
         * \brief The number of calls to \p observer::will_update
         */
        int changed_dependencies = 0;

        /**
         * \brief Have any of the dependency cells actually changed?
         */
        int has_changed = false;
    };

}  // live_cells

#endif /* LIVE_CELLS_OBSERVER_CELL_STATE_HPP */
