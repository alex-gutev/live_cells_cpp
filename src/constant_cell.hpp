#ifndef LIVE_CELLS_CONSTANT_CELL_HPP
#define LIVE_CELLS_CONSTANT_CELL_HPP

#include <functional>
#include <memory>

#include "cell.hpp"

namespace live_cells {

    /**
     * Key identifying a constant cell.
     */
    template <typename T>
    struct constant_key : key {
        /** Constant value */
        const T value;

        /**
         * Create a constant cell key for a given value.
         *
         * @param value The value
         */
        constant_key(T value) : value(value) {}

        bool eq(const key &k) const noexcept override {
            auto *ptr = dynamic_cast<const constant_key*>(&k);

            return ptr != nullptr && value == ptr->value;
        }

        std::size_t hash() const noexcept override {
            return std::hash<T>{}(value);
        }
    };

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
        constant_cell(const T &value) :
            cell<T>(std::make_shared<constant_key<T>>(value)),
            m_value(value) {}

        T value() const override {
            return m_value;
        }

        void add_observer(observer::ref) override {
            // Do nothing
        }

        void remove_observer(observer::ref) override {
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
