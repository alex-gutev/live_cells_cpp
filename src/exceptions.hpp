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

#ifndef LIVE_CELLS_EXCEPTIONS_HPP
#define LIVE_CELLS_EXCEPTIONS_HPP

#include <exception>

namespace live_cells {

    /**
     * \brief Exception indicating that the value of a cell should not
     * be computed.
     *
     * When this exception is thrown inside the value computation
     * function of a computed cell, the cell's value is not
     * updated. Instead its current value is preserved.
     */
    struct stop_compute_exception : std::exception {
        const char *what() const noexcept override {
            return "Stop computation of the value of a cell.";
        }
    };

    /**
     * \brief Exception indicating that the value of a cell was
     * referenced before its initial value was computed.
     */
    struct uninitialized_cell_error : std::exception {
        const char *what() const noexcept override {
            return "The value of a cell was referenced before it was initialized.";
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_EXCEPTIONS_HPP */
