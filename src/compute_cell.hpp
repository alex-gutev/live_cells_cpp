#ifndef LIVE_CELLS_COMPUTE_CELL_HPP
#define LIVE_CELLS_COMPUTE_CELL_HPP

#include "dependent_cell.hpp"

namespace live_cells {

    /**
     * A cell with a value that is a function of one or more argument
     * cells.
     *
     * The value computation function is provided to this class on
     * construction.
     */
    template <typename T, typename... Ts>
    class compute_cell : dependent_cell<T, Ts...> {
    public:
        /**
         * Create a cell with a value that is a function of the values
         * of the cells @a args.
         *
         * @param compute A function of no arguments, that is called
         *   to compute the cell's value when necessary.
         *
         * @param args Argument cell observables
         */
        compute_cell(std::function<T()> compute, Ts... args) :
            dependent_cell<T, Ts...>(args...),
            compute(compute) {}

        T value() const {
            return compute();
        }

    private:
        const std::function<T()> compute;

    };

    /**
     * Create a cell with a value that is a function of the values of
     * one or more argument cells.
     *
     * @param compute Value computation function. This function is
     *   called to compute the value of the cell when necessary. The
     *   values of the cells @a args are passed to @a compute in the
     *   same order as they are provided in this function.
     *
     * @param args Argument cells
     */
    template <typename F, typename... Args>
    auto computed(F&& compute, Args&&... args) -> compute_cell<decltype(compute(args.value()...)), Args...> {
        return compute_cell<decltype(compute(args.value()...)), Args...>(
            [compute, args...] {
                return compute(args.value()...);
            },
            args...
         );
    }

}  // live_cells

#endif /* LIVE_CELLS_COMPUTE_CELL_HPP */
