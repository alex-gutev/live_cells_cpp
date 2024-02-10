#ifndef LIVE_CELLS_EQUALITY_HPP
#define LIVE_CELLS_EQUALITY_HPP

#include "computed.hpp"

namespace live_cells {

    /**
     * Create a cell which compares two cells for equality by ==.
     *
     * @param a A cell
     * @param b A cell
     *
     * @return A computed cell which evaluates the expression a == b.
     */
    template <typename T, typename U>
    auto operator ==(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a == b;
        });
    }

    /**
     * Create a cell which compares two cells for inequality by !=.
     *
     * @param a A cell
     * @param b A cell
     *
     * @return A computed cell which evaluates the expression a != b.
     */
    template <typename T, typename U>
    auto operator !=(const T &a, const U &b) {
        return computed(a, b, [] (auto a, auto b) {
            return a != b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_EQUALITY_HPP */
