#ifndef LIVE_CELLS_MAYBE_CELL_HPP
#define LIVE_CELLS_MAYBE_CELL_HPP

#include <concepts>

#include "keys.hpp"
#include "maybe.hpp"
#include "types.hpp"
#include "computed.hpp"
#include "mutable_computed.hpp"

namespace live_cells {

    /**
     * \brief Key identifying cells created with \p maybe_cell
     */
    template<typename T>
    struct maybe_cell_key : value_key<T> {
        using value_key<T>::value_key;
    };

    /**
     * \brief Key identifying cells created with the mutable cell
     * overload of \p maybe_cell.
     */
    template<typename T>
    struct mutable_maybe_cell_key : value_key<T> {
        using value_key<T>::value_key;
    };



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
        return computed(key_ref::create<maybe_cell_key<key_ref>>(cell.key()), [=] {
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
        return mutable_computed(key_ref::create<mutable_maybe_cell_key<key_ref>>(cell.key()), [=] {
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

    namespace ops {
        /**
         * \brief Cell operator that creates a maybe cell as if by \p
         * live_cells::maybe_cell.
         */
        struct maybe_operator {};

        /**
         * \brief Cell operator that creates a maybe cell as if by \p
         * live_cells::maybe_cell.
         *
         * Usage:
         *
         * \code{.cpp}
         * auto b = a | maybe;
         * \endcode
         *
         * This is equivalent to:
         *
         * \code{.cpp}
         * auto b = live_cells::maybe_cell(a);
         * \endcode
         */
        constexpr maybe_operator maybe{};

        inline auto operator|(const Cell auto &cell, const maybe_operator &) {
            return maybe_cell(cell);
        }

        inline auto operator|(const MutableCell auto &cell, const maybe_operator &) {
            return maybe_cell(cell);
        }
    }
}  // live_cells

#endif /* LIVE_CELLS_MAYBE_CELL_HPP */
