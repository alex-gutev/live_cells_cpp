#ifndef LIVE_CELLS_MAYBE_CELL_HPP
#define LIVE_CELLS_MAYBE_CELL_HPP

#include <concepts>

#include "maybe.hpp"
#include "types.hpp"
#include "computed.hpp"
#include "mutable_computed.hpp"

namespace live_cells {

    /**
     * \brief Create a cell that wraps the value of a \a cell in a \p
     * maybe.
     *
     * If \a cell throws an exception, the returned cell evaluates to
     * a \p maybe holding the exception.
     *
     * \param cell The cell
     *
     * \return A cell that wraps the value of \a cell in a \p maybe.
     */
    auto maybe_cell(const Cell auto &cell) {
        return computed([=] {
            return maybe_wrap(cell);
        });
    }

    /**
     * \brief Create a cell that wraps the value of a mutable \a cell
     * in a \p maybe.
     *
     * If \a cell throws an exception, the returned cell evaluates to
     * a \p maybe holding the exception.
     *
     * A mutable cell is created, which when assigned a \p maybe that
     * wraps a value, the value of \a cell is assigned to the value
     * held in the \p maybe. When the cell is assigned a \p maybe
     * holding an exception, the value of \a cell is not changed.
     *
     * \param cell The cell
     *
     * \return A mutable cell that wraps the value of \a cell in a \p
     * maybe.
     */
    auto maybe_cell(const MutableCell auto &cell) {
        return mutable_computed([=] {
            return maybe_wrap(cell);
        }, [=] (auto value) {
            try {
                cell = value.unwrap();
            }
            catch (...) {
                // Prevent exception from propagating to caller
            }
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_MAYBE_CELL_HPP */
