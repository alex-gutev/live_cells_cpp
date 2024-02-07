#ifndef LIVE_CELLS_MUTABLE_COMPUTED_HPP
#define LIVE_CELLS_MUTABLE_COMPUTED_HPP

#include <utility>

#include "static_mutable_compute_cell.hpp"
#include "dynamic_mutable_compute_cell.hpp"

namespace live_cells {

    /**
     * Create a mutable computed cell.
     *
     * Ordinarily the value of the cell is the value computed by @a fn
     * which is a function of the cells in @a args.
     *
     * The returned cell is also a mutable cell, which means its value
     * can be set explicitly. When the value of the cell is set
     * explicitly, the reverse computation function @a reverse is
     * called. The function @a reverse, should set the values of the
     * cells in @a args such that the compute value function @a fn
     * returns the same value as the value that was assigned to the
     * cell.
     *
     * @param fn Compute value function.
     *
     *   This function is passed the values of the argument cells in
     *   @a args, in the same order as they are provided in the
     *   argument list.
     *
     * @param reverse Reverse computation function.
     *
     *   This function is passed the value that was assigned to the
     *   cell.
     *
     * @param args Arguments to @a compute.
     *
     * @return The cell.
     */
    template <typename F, typename R, typename... Args>
    auto mutable_computed(F fn, R&& reverse, Args... args) {
        return static_mutable_compute_cell<decltype(fn(args.value()...))>(
            [fn, args...] () {
                return fn(args.value()...);
            },
            std::forward<R>(reverse),
            std::forward<Args>(args)...
        );
    }

    /**
     * Create a mutable computed cell.
     *
     * This differs from the other overload in that the argument cells
     * are not specified explicitly, but are determined
     * dynamically.
     *
     * The compute value function @a compute should use the `use()`
     * function to retrieve the values of its arguments.
     *
     * @param fn Compute value function.
     *
     *   This function is called with no arguments. The values of the
     *   argument cells should be referenced with `use()`.
     *
     * @param reverse Reverse computation function.
     *
     *   This function is passed the value that was assigned to the
     *   cell.
     *
     * @return The cell.
     */
    template <typename F, typename R>
    auto mutable_computed(F&& fn, R&& reverse) {
        return dynamic_mutable_compute_cell<decltype(fn())>(
            std::forward<F>(fn),
            std::forward<R>(reverse)
        );
    }

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_COMPUTED_HPP */
