#ifndef LIVE_CELLS_COMPUTED_HPP
#define LIVE_CELLS_COMPUTED_HPP

#include <tuple>
#include <utility>

#include "compute_cell.hpp"
#include "dynamic_compute_cell.hpp"

namespace live_cells {
    /**
     * Namespace for private definitions
     */
    namespace internal {

        /**
         * Create a compute_cell with compute function @a fn called on
         * the arguments in the tuple @a args.
         *
         * @param fn   Compute value function.
         * @param args Tuple holding arguments to @a fn
         *
         * @return A compute_cell
         */
        template <typename F, typename Tuple, std::size_t... I>
        auto unpack_compute_args(F fn, Tuple args, std::index_sequence<I...>) {
            typedef decltype(fn(std::get<I>(args).value()...)) value_type;

            return compute_cell<value_type, typename std::tuple_element<I, Tuple>::type...>(
                [fn, args] {
                    return fn(std::get<I>(args).value()...);
                },
                std::get<I>(args)...
            );
        }

        /**
         * Create a compute_cell with compute function @a fn called on
         * the arguments in the tuple @a args.
         *
         * @param key  Key identifying the cell.
         * @param fn   Compute value function.
         * @param args Tuple holding arguments to @a fn
         *
         * @return A compute_cell
         */
        template <typename F, typename Tuple, std::size_t... I>
        auto unpack_compute_args(key_ref key, F fn, Tuple args, std::index_sequence<I...>) {
            typedef decltype(fn(std::get<I>(args).value()...)) value_type;

            return compute_cell<value_type, typename std::tuple_element<I, Tuple>::type...>(
                key,
                [fn, args] {
                    return fn(std::get<I>(args).value()...);
                },
                std::get<I>(args)...
            );
        }

        template <typename F, typename Tuple>
        auto unpack_compute_args(F fn, Tuple args) {
            constexpr auto size = std::tuple_size<Tuple>::value;
            return unpack_compute_args(fn, args, std::make_index_sequence<size>{});
        }

        template <typename F, typename Tuple>
        auto unpack_compute_args(key_ref key, F fn, Tuple args) {
            constexpr auto size = std::tuple_size<Tuple>::value;
            return unpack_compute_args(key, fn, args, std::make_index_sequence<size>{});
        }

        template <typename Tuple, typename F>
        auto make_compute_cell(Tuple args, F fn) {
            return unpack_compute_args(fn, args);
        }

        template <typename Tuple, typename F>
        auto make_compute_cell(key_ref key, Tuple args, F fn) {
            return unpack_compute_args(key, fn, args);
        }

        /**
         * Create a compute_cell.
         *
         * @param packed compute_cell arguments till this point packed in a tuple.
         *
         * @param arg1   First argument to compute function.
         *
         * @param args   Remaining arguments followed by the compute function.
         *
         * @return A compute_cell
         */
        template <typename Tuple, typename A, typename... As>
        auto make_compute_cell(Tuple packed, A arg1, As... args) {
            return make_compute_cell(std::tuple_cat(packed, std::make_tuple(arg1)), args...);
        }

        /**
         * Create a compute_cell.
         *
         * @param packed compute_cell arguments till this point packed in a tuple.
         *
         * @param arg1   First argument to compute function.
         *
         * @param args   Remaining arguments followed by the compute function.
         *
         * @return A compute_cell
         */
        template <typename Tuple, typename A, typename... As>
        auto make_compute_cell(key_ref key, Tuple packed, A arg1, As... args) {
            return make_compute_cell(key, std::tuple_cat(packed, std::make_tuple(arg1)), args...);
        }

    }  // internal

    /**
     * Create a cell with a value that is a function of one or more
     * argument cells.
     *
     * The argument cells, referenced by @a compute, are determined
     * dynamically. For this to work, the values of the argument cells
     * must be referenced using live_cells::use() within @a compute().
     *
     * @param compute Compute value function
     *
     * @return The cell
     */
    template <typename F>
    auto computed(F&& compute) {
        return dynamic_compute_cell<decltype(compute())>(std::forward<F>(compute));
    }

    /**
     * Create a cell with a value that is a function of the values of
     * one or more argument cells.
     *
     * @param arg1 First argument to value computation function.
     *
     * @param args Remaining arguments followed by compute value function.
     *
     *   The compute value function is called to compute the value of
     *   the cell when necessary. The values of the argument cells are
     *   passed to the compute value function in the same order as
     *   they are provided in this function.
     *
     * @return The cell
     */
    template <typename A, typename... As>
    auto computed(A arg1, As... args) {
        return internal::make_compute_cell(std::make_tuple(), arg1, args...);
    }

    /**
     * Create a cell with a value that is a function of the values of
     * one or more argument cells.
     *
     * @param key Key identifying the cell
     *
     * @param arg1 First argument to value computation function.
     *
     * @param args Remaining arguments followed by compute value function.
     *
     *   The compute value function is called to compute the value of
     *   the cell when necessary. The values of the argument cells are
     *   passed to the compute value function in the same order as
     *   they are provided in this function.
     *
     * @return The cell
     */
    template <typename A, typename... As>
    auto computed(key_ref key, A arg1, As... args) {
        return internal::make_compute_cell(key, std::make_tuple(), arg1, args...);
    }

}  // live_cells

#endif /* LIVE_CELLS_COMPUTED_HPP */
