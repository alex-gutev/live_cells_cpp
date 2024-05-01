#ifndef LIVE_CELLS_OBSERVER_CELL_STATE_HPP
#define LIVE_CELLS_OBSERVER_CELL_STATE_HPP

#include <cassert>
#include <concepts>

namespace live_cells {

    /**
     * Concept specifying the base interface of the state of a cell
     * that observes another cell.
     *
     * The following methods must be implemented:
     *
     * - \c did_change()
     *
     *   Should return \c true if the value of the cell may have
     *   changed, or \c false if it is known that the value of the
     *   cell has not changed.
     *
     * - \c pre_update()
     *
     *   Called when the cell is first notified that the value of one
     *   of its dependencies will change.
     *
     * - \c post_update()
     *
     *   Called after the value of the cell has been updated. This
     *   method is not called if the value of the cell has not
     *   changed.
     */
    template <typename T>
    concept ObserverCellStateBase = requires (T s) {
        { s.did_change() } -> std::same_as<bool>;
        { s.pre_update() };
        { s.post_update() };
    };

    /**
     * Default base of a cell that observes another cell.
     *
     * This provides default implementations of the \p did_change(),
     * \p pre_update() and \p post_update() methods.
     */
    struct observer_cell_state_base {
        bool did_change() {
            return true;
        }

        void pre_update() {}
        void post_update() {}
    };

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
     *
     * \a Base is the base to use for this observer_cell_state, which
     * defines the \p did_change(), \p pre_update() and \p
     * post_update() methods.
     *
     * \note \a Base is not the base class of this class.
     */
    template <ObserverCellStateBase Base = observer_cell_state_base>
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

                base.pre_update();

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

                    notify_update(has_changed && base.did_change());
                    updating = false;

                    if (has_changed) {
                        base.post_update();
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

        /**
         * Defines the \p did_change(), \p pre_update() and \p
         * post_update methods.
         */
        Base base;
    };

}  // live_cells

#endif /* LIVE_CELLS_OBSERVER_CELL_STATE_HPP */
