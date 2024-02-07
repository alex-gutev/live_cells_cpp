#ifndef LIVE_CELLS_MUTABLE_COMPUTED_HPP
#define LIVE_CELLS_MUTABLE_COMPUTED_HPP

#include <tuple>
#include <utility>

#include "static_mutable_compute_cell.hpp"
#include "dynamic_mutable_compute_cell.hpp"

namespace live_cells {
    /**
     * Namespace for private definitions
     */
    namespace internal {

        /**
         * Create a static_mutable_compute_cell with compute function
         * @a compute called on the arguments in the tuple @a args,
         * and reverse compute function @a reverse.
         *
         * @param compute Compute value function.
         * @param reverse Reverse compute function.
         * @param args    Tuple holding arguments to @a fn
         *
         * @return A static_mutable_compute_cell
         */
        template <typename C, typename R, typename Tuple, std::size_t... I>
        auto unpack_mutable_compute_args(C compute, R reverse, Tuple args, std::index_sequence<I...>) {
            typedef decltype(compute(std::get<I>(args).value()...)) value_type;

            auto fn = [compute, args] {
                return compute(std::get<I>(args).value()...);
            };

            return static_mutable_compute_cell<value_type>(fn, reverse, std::get<I>(args)...);
        }

        template <typename Tuple, typename C, typename R>
        auto make_mutable_compute_cell(Tuple args, C compute, R reverse) {
            constexpr auto size = std::tuple_size<Tuple>::value;
            return unpack_mutable_compute_args(compute, reverse, args, std::make_index_sequence<size>());
        }

        /**
         * Create a static_mutable_compute_cell.
         *
         * @param packed cell arguments till this point packed in a tuple.
         *
         * @param arg1   First argument to compute function.
         *
         * @param args    Remaining arguments followed by the compute,
         *    and reverse compute functions.
         *
         * @return A static_mutable_compute_cell
         */
        template <typename Tuple, typename A, typename... As>
        auto make_mutable_compute_cell(Tuple packed, A arg1, As... args) {
            return make_mutable_compute_cell(std::tuple_cat(packed, std::make_tuple(arg1)), args...);
        }

    }  // internal

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
     * @param arg1 First argument to compute value function.
     *
     * @param arg2, args Remaining compute value function arguments
     *    followed by compute value and reverse compute functions.
     *
     *   The compute value function is passed the values of the
     *   argument cells, in the same order as they are provided in the
     *   argument list.
     *
     *   The reverse compute function (last in the argument list) is
     *   passed the value that was assigned to the cell.
     *
     * @return The cell.
     */
    template <typename A1, typename A2, typename... As>
    auto mutable_computed(A1 arg1, A2 arg2, As... args) {
        return internal::make_mutable_compute_cell(std::make_tuple(), arg1, arg2, args...);
    }

}  // live_cells

#endif /* LIVE_CELLS_MUTABLE_COMPUTED_HPP */
