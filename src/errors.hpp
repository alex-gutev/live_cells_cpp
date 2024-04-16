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

#ifndef LIVE_CELLS_ERRORS_HPP
#define LIVE_CELLS_ERRORS_HPP

#include "observable.hpp"
#include "computed.hpp"
#include "store_cell.hpp"

namespace live_cells {
    /**
     * \brief Create a Cell which handles all exceptions thrown while
     * computing the value of a \a cell.
     *
     * If an exception is thrown during the computation of the value
     * of \a cell, the cell evaluates to the value of the \a error_value
     * cell.
     *
     * \param cell        The cell for which to handle exceptions
     *
     * \param error_value The cell holding the value to use when an
     *    exception is thrown.
     *
     * \return The new cell
     */
    auto on_error(const Cell auto &cell, const Cell auto &error_value) {
        return store(
            make_compute_cell([=] () {
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
     * \brief Create a Cell which handles exceptions of type \a E
     * thrown while computing the value of a \a cell.
     *
     * If an exception is thrown during the computation of the value
     * of \a cell, the cell evaluates to the value of the \a error_value
     * cell.
     *
     * \note Type \a E cannot be deduced so it should be specified
     * explicitly.
     *
     * \param cell        The cell for which to handle exceptions
     *
     * \param error_value The cell holding the value to use when an
     *    exception is thrown.
     *
     * \return The new cell
     */
    template <typename E>
    auto on_error(const Cell auto &cell, const Cell auto &error_value) {
        return store(
            make_compute_cell([=] () {
                try {
                    return cell.value();
                }
                catch (const E &) {
                    return error_value.value();
                }
            }, cell, error_value)
        );
    }

    namespace ops {

        /**
         * \brief Create an \p Operator for handling exceptions thrown by
         * the operand \p Cell.
         *
         * \see live_cells::on_error
         *
         * \param error_value The cell holding the value to use when
         *   an exception is thrown by the operand cell.
         *
         * \return The error handling \p Operator.
         */
        auto on_error(const Cell auto &error_value) {
            return [&] (const Cell auto &cell) {
                return live_cells::on_error(cell, error_value);
            };
        }

        /**
         * \brief Create an \p Operator for handling exceptions of
         * type \a E thrown by the operand \p Cell.
         *
         * \see live_cells::on_error
         *
         * \param error_value The cell holding the value to use when
         *   an exception is thrown by the operand cell.
         *
         * \return The error handling \p Operator.
         */
        template <typename E>
        auto on_error(const Cell auto &error_value) {
            return [&] (const Cell auto &cell) {
                return live_cells::on_error<E>(cell, error_value);
            };
        }

    }  // operators

}  // live_cells

#endif /* LIVE_CELLS_ERRORS_HPP */
