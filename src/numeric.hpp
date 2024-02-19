#ifndef LIVE_CELLS_NUMERIC_HPP
#define LIVE_CELLS_NUMERIC_HPP

#include "cell.hpp"
#include "computed.hpp"

namespace live_cells {
    /**
     * Returns a computed cell that evaluates `a + b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator +(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a + b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a - b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator -(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a - b;
        });
    }

    /**
     * Returns a computed cell that evaluates to `-cell`, on the value
     * held in `cell`.
     */
    template <typename T, typename = enable_if_observable<T>>
    auto operator -(T cell) {
        return computed(cell, [] (auto value) {
            return -value;
        });
    }

    /**
     * Returns a computed cell that evaluates `a * b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator *(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a * b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a / b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator /(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a / b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a % b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator %(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a % b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a < b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator <(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a < b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a <= b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator <=(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a <= b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a > b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator >(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a > b;
        });
    }

    /**
     * Returns a computed cell that evaluates `a >= b` on the values
     * held in cells `a` and `b`.
     */
    template <typename T,
              typename U,
              typename = enable_if_observable<T>,
              typename = enable_if_observable<U>>
    auto operator >=(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a >= b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_NUMERIC_HPP */
