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
            handle_will_update([] {}, notify_will_update);
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
         * \param pre_update A function of no arguments that is called
         *    before the update cycle begins.\n\n This function is
         *    only called if this method is being called for the first
         *    time during the current update cycle.
         *
         * \param notify_will_update Function that notifies the
         *    observers of the cell.
         */
        void handle_will_update(const std::invocable auto &pre_update,
                                const std::invocable auto &notify_will_update) {
            if (!updating) {
                assert(changed_dependencies == 0 && "Number of changed dependencies == 0 at start of update cycle.");

                pre_update();

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
         * \param changed Has the value of the dependency cell changed?
         *
         * \param notify_will_update Function that notifies the
         *    observers of the cell.\n The argument passed to this
         *    cell is true if the cell's value may have changed or
         *    false if it has not changed.
         */
        void handle_update(bool changed, const std::invocable<bool> auto &notify_update) {
            handle_update(changed, [] { return true; }, notify_update, [] { });
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
         * \param changed Has the value of the dependency cell changed?
         *
         * \param did_change Function called with no arguments, to
         *    determine whether the value of the cell has changed.\n\n
         *    This function should return \c true if the value of the
         *    cell may have changed, \c false if it is know that the
         *    cell value has not changed.
         *
         * \param notify_will_update Function that notifies the
         *    observers of the cell.\n The argument passed to this
         *    cell is true if the cell's value may have changed or
         *    false if it has not changed.
         *
         * \param post_update A function of no arguments that is called
         *    after the update cycle ends.\n\n This function is only
         *    called if this method is being called for the last
         *    dependency cell during this update cycle and the value
         *    of this cell has changed.
         */
        void handle_update(bool changed,
                           const std::invocable auto &did_change,
                           const std::invocable<bool> auto &notify_update,
                           const std::invocable auto &post_update) {
            if (updating) {
                assert(changed_dependencies > 0 && "Calls to update() are not more than calls to will_update().");

                has_changed = has_changed || changed;

                if (--changed_dependencies == 0) {
                    stale = stale || has_changed;

                    notify_update(has_changed && did_change());
                    updating = false;

                    if (has_changed) {
                        post_update();
                    }
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
