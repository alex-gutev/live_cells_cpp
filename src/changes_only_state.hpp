#ifndef LIVE_CELLS_CHANGES_ONLY_STATE_HPP
#define LIVE_CELLS_CHANGES_ONLY_STATE_HPP

#include <optional>

#include "compute_state.hpp"

namespace live_cells {

    /**
     * \brief Mixin that modifies a \p compute_cell_state subclass to
     * only notify its observers if the new value of the cell is not
     * equal (!=) to the previous value.
     *
     * \a Base is a \p compute_cell_state subclass, which this class
     * inherits.
     *
     * \note This mixin converts the evaluation semantics of the cell
     * from lazy, computed only when the value is actually referenced,
     * to eager, computed whenever the value of an argument cell
     * changes.
     */
    template <typename Base>
    class changes_only_cell_state : public Base {
    public:
        using Base::Base;

        void will_update(const key_ref &k) override {
            Base::handle_will_update([this] { pre_update(); },
                                     [this] { Base::notify_will_update(); });
        }

        void update(const key_ref &k, bool changed) override {
            Base::handle_update(changed,
                                [this] { return did_change(); },
                                [this] (bool changed) { Base::notify_update(changed); },
                                [this] { post_update(); });
        }

    private:
        /**
         * \brief The previous value
         */
        std::optional<typename Base::value_type> old;

        /**
         * \brief Check if the current value of the cell is equal to
         * the previous value.
         *
         * \return \p false if the current value is \p != to the
         *   previous value.
         */
        bool did_change() {
            try {
                return !old.has_value() || Base::value() != old.value();
            }
            catch (...) {
                old.reset();
            }

            return true;
        }

        /**
         * \brief Record the value of the cell before it is updated.
         *
         * This should be called at the start of the update cycle.
         */
        void pre_update() {
            try {
                old = Base::value();
            }
            catch (...) {
                old.reset();
            }
        }

        /**
         * \brief Reset the previous value of the cell after it is
         * updated.
         *
         * This should be called after the end of the update cycle.
         */
        void post_update() {
            old.reset();
        }
    };

    /**
     * \brief Cell option specifying that the cell, to which it is
     * applied, should only notify its observers when its new value is
     * not equal (by \c ==) to its previous value.
     */
    struct changes_only_option {};

    /**
     * \brief Cell option specifying that the cell, to which it is
     * applied, should only notify its observers when its new value is
     * not equal (by \c ==) to its previous value.
     */
    constexpr changes_only_option changes_only{};

}  // live_cells

#endif /* LIVE_CELLS_CHANGES_ONLY_STATE_HPP */
