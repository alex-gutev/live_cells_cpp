#ifndef LIVE_CELLS_CONSTANT_CELL_HPP
#define LIVE_CELLS_CONSTANT_CELL_HPP

#include "cell.hpp"

namespace live_cells {

    /**
     * A cell which holds a constant value
     *
     * Adding observers to this cell has no effect since its value
     * never changes.
     */
    template <typename T>
    class constant_cell : public cell<T> {
    public:
        /**
         * Construct a cell with a constant value.
         *
         * @param value The constant value
         */
        constant_cell(const T &value) : m_value(value) {}

        virtual T value() const override {
            return m_value;
        }

        virtual void add_observer(observer::ref) override {
            // Do nothing
        }

        virtual void remove_observer(observer::ref) override {
            // Do nothing
        }

    private:
        const T m_value;

    };

    /**
     * Create a cell which holds a constant value.
     *
     * @param value The constant value.
     *
     * @return A cell
     */
    template <typename T>
    constant_cell<T> value_cell(const T &value) {
        return constant_cell<T>(value);
    }

}  // live_cells

#endif /* LIVE_CELLS_CONSTANT_CELL_HPP */
