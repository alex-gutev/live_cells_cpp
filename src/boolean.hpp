#ifndef LIVE_CELLS_BOOLEAN_HPP
#define LIVE_CELLS_BOOLEAN_HPP

#include "observable.hpp"
#include "computed.hpp"
#include "store_cell.hpp"

namespace live_cells {
    /**
     * Returns a cell which evaluates to the logical and of the values
     * of cells `a` and `b`.
     *
     * NOTE: This operator is short-circuiting which means that the
     * value of cell `b` is only referenced if `a` is true.
     */
    auto operator && (const Cell auto &a, const Cell auto &b) {
        return make_compute_cell([=] () {
            return a.value() && b.value();
        }, a, b);
    }

    /**
     * Returns a cell which evaluates to the logical or of the values
     * of cells `a` and `b`.
     *
     * NOTE: This operator is short-circuiting which means that the
     * value of cell `b` is only referenced if `a` is false.
     */
    auto operator || (const Cell auto &a, const Cell auto &b) {
        return make_compute_cell([=] () {
            return a.value() || b.value();
        }, a, b);
    }

    /**
     * Returns a cell which evaluates to the logical not of the value
     * of a cell.
     */
    auto operator ! (const Cell auto &cell) {
        return computed(cell, [] (auto value) {
            return !value;
        });
    }

    /**
     * Create a cell which selects between the values of two cells
     * based on the value of a condition cell.
     *
     * If the value of the `condition` cell is false, the cell
     * evaluates to the value of the `if_true` cell. Otherwise the
     * cell evaluates to the value of the `if_false` cell.
     */
    auto select(Cell auto condition, Cell auto if_true, Cell auto if_false) {
        return make_compute_cell([=] () {
            return condition.value() ? if_true.value() : if_false.value();
        }, condition, if_true, if_false);
    }

    /**
     * Create a cell which evaluates to the value of another cell only
     * if a condition is true.
     *
     * If the value of the `condition` cell is false, the cell
     * evaluates to the value of the `if_true` cell. Otherwise the
     * cell preserves its current value as if by calling `none()`.
     */
    auto select(Cell auto condition, Cell auto if_true) {
        return store(
            make_compute_cell([=] () {
                if (!condition.value()) {
                    none();
                }

                return if_true.value();
            }, condition, if_true)
        );
    }

}  // live_cells

#endif /* LIVE_CELLS_BOOLEAN_HPP */
