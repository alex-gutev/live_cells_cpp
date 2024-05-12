/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You
 * may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef LIVE_CELLS_NUMERIC_HPP
#define LIVE_CELLS_NUMERIC_HPP

#include "constant_cell.hpp"
#include "observable.hpp"
#include "computed.hpp"

namespace live_cells {
    /**
     * \brief Create a \p Cell that computes the sum (using the \c +
     * operator) of cells \a and \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator +(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a + b;
        });
    }

    /**
     * \brief Create a \p Cell that computes the difference (using the
     * \c - operator) of cells \a and \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator -(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a - b;
        });
    }

    /**
     * \brief Create a \p Cell that negates the value of another cell
     * using the unary \c - operator.
     *
     * \param cell The \p Cell to negate.
     *
     * \return A new \p Cell
     */
    auto operator -(const Cell auto &cell) {
        return computed(cell, [] (auto value) {
            return -value;
        });
    }

    /**
     * \brief Create a \p Cell that computes the product (using the \c
     * * operator) of cells \a and \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator *(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a * b;
        });
    }

    /**
     * \brief Create a \p Cell that computes the quotient (using the
     * \c / operator) of cells \a and \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator /(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a / b;
        });
    }

    /**
     * \brief Create a \p Cell that computes the remainder (using the
     * \c % operator) of cell \a divided by \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator %(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a % b;
        });
    }

    /**
     * \brief Create a \p Cell that is \p true if the value of cell \a
     * is less than (by the \c < operator) the value of cell \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator <(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a < b;
        });
    }

    /**
     * \brief Create a \p Cell that is \p true if the value of cell \a
     * is less than or equal to (by the \c <= operator) the value of
     * cell \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator <=(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a <= b;
        });
    }

    /**
     * \brief Create a \p Cell that is \p true if the value of cell \a
     * is greater than (by the \c > operator) the value of cell \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator >(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a > b;
        });
    }

    /**
     * \brief Create a \p Cell that is \p true if the value of cell \a
     * is greater than or equal to (by the \c >= operator) the value
     * of cell \b.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A new \p Cell
     */
    template <typename A, typename B>
    auto operator >=(const A &a, const B &b) requires (
        (Cell<A> && CellOrValue<B>) ||
        (CellOrValue<A> && Cell<B>)
    ) {
        return computed(ensure_cell(a), ensure_cell(b), [] (auto a, auto b) {
            return a >= b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_NUMERIC_HPP */
