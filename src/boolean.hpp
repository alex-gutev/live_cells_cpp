#ifndef LIVE_CELLS_BOOLEAN_HPP
#define LIVE_CELLS_BOOLEAN_HPP

#include "cell.hpp"
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
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator && (const T &a, const U &b) {
        return compute_cell<bool, T, U>([=] () {
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
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator || (const T &a, const U &b) {
        return compute_cell<bool, T, U>([=] () {
            return a.value() || b.value();
        }, a, b);
    }

    /**
     * Returns a cell which evaluates to the logical not of the value
     * of a cell.
     */
    template <typename T,
              typename = enable_if_observable<T>>
    auto operator ! (const T &cell) {
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
    template <typename T, typename U, typename V>
    auto select(T condition, U if_true, V if_false) {
        return compute_cell<typename U::value_type, T, U, V>([=] () {
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
    template <typename T, typename U>
    auto select(T condition, U if_true) {
        return store(
            compute_cell<typename U::value_type, T, U>([=] () {
                if (!condition.value()) {
                    none();
                }

                return if_true.value();
            }, condition, if_true)
        );
    }

}  // live_cells

#endif /* LIVE_CELLS_BOOLEAN_HPP */
