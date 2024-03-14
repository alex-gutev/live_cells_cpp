#ifndef LIVE_CELLS_NUMERIC_HPP
#define LIVE_CELLS_NUMERIC_HPP

#include "observable.hpp"
#include "computed.hpp"

namespace live_cells {
    /**
     * Returns a computed cell that evaluates `a + b` on the values
     * held in cells `a` and `b`.
     */
    auto operator +(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a + b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a - b` on the values
     * held in cells `a` and `b`.
     */
    auto operator -(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a - b;
        });
    }

    /**
     * Returns a computed cell that evaluates to `-cell`, on the value
     * held in `cell`.
     */
    auto operator -(const Cell auto &cell) {
        return computed(cell, [] (auto value) {
            return -value;
        });
    }

    /**
     * Returns a computed cell that evaluates `a * b` on the values
     * held in cells `a` and `b`.
     */
    auto operator *(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a * b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a / b` on the values
     * held in cells `a` and `b`.
     */
    auto operator /(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a / b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a % b` on the values
     * held in cells `a` and `b`.
     */
    auto operator %(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a % b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a < b` on the values
     * held in cells `a` and `b`.
     */
    auto operator <(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a < b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a <= b` on the values
     * held in cells `a` and `b`.
     */
    auto operator <=(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a <= b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a > b` on the values
     * held in cells `a` and `b`.
     */
    auto operator >(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a > b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a >= b` on the values
     * held in cells `a` and `b`.
     */
    auto operator >=(const Cell auto &a, const Cell auto &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a >= b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_NUMERIC_HPP */
