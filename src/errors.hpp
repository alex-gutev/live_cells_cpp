#ifndef LIVE_CELLS_ERRORS_HPP
#define LIVE_CELLS_ERRORS_HPP

#include "cell.hpp"
#include "computed.hpp"
#include "store_cell.hpp"

namespace live_cells {
    /**
     * Create a cell which handles all exceptions thrown while
     * computing the value of a cell.
     *
     * If an exception is thrown during the computation of the value
     * of `cell`, the cell evaluates to the value of the `error_value`
     * cell.
     *
     * @param cell        The cell for which to handle exceptions
     *
     * @param error_value The cell holding the value to use if an
     *    exception is thrown.
     *
     * @return The new cell
     */
    template <typename T, typename U>
    auto on_error(const T &cell, const U &error_value) {
        return store(
            compute_cell<typename T::value_type, T, U>([=] () {
                try {
                    return cell.value();
                }
                catch (...) {
                    return error_value.value();
                }
            }, cell, error_value)
        );
    }

    /**
     * Like the other overload but only handles exceptions of type
     * `E`, which is the first template type parameter.
     *
     * Type `E` cannot be deduced so it should be specified
     * explicitly.
     */
    template <typename E, typename T, typename U>
    auto on_error(const T &cell, const U &error_value) {
        return store(
            compute_cell<typename T::value_type, T, U>([=] () {
                try {
                    return cell.value();
                }
                catch (const E &) {
                    return error_value.value();
                }
            }, cell, error_value)
        );
    }

}  // live_cells

#endif /* LIVE_CELLS_ERRORS_HPP */
