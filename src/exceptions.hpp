#ifndef LIVE_CELLS_EXCEPTIONS_HPP
#define LIVE_CELLS_EXCEPTIONS_HPP

#include <exception>

namespace live_cells {

    /**
     * Indicates that the value of a cell should not be computed.
     *
     * When this exception is thrown inside a cell's value computation
     * function, the cell's value is not updated. Instead it's current
     * value is preserved.
     */
    struct stop_compute_exception : std::exception {
        const char *what() const noexcept override {
            return "Stop computation of the value of a cell.";
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_EXCEPTIONS_HPP */
