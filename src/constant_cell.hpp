/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LIVE_CELLS_CONSTANT_CELL_HPP
#define LIVE_CELLS_CONSTANT_CELL_HPP

#include <functional>
#include <memory>

#include "observable.hpp"

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
    class constant_cell {
    public:
        typedef T value_type;

        /**
         * Construct a cell with a constant value.
         *
         * @param value The constant value
         */
        constant_cell(const T &value) : m_value(value) {}

        key_ref key() const {
            return key_ref::create<constant_key<T>>(m_value);
        }

        T value() const {
            return m_value;
        }

        void add_observer(observer::ref) const {
            // Do nothing
        }

        void remove_observer(observer::ref) const {
            // Do nothing
        }

        T operator()() const {
            // No need to track a constant cell
            return m_value;
        }

    private:
        const T m_value;

    };

    /**
     * Create a cell that holds a constant value.
     *
     * @param value The constant value.
     *
     * @return A cell
     */
    template <typename T>
    constant_cell<T> value(const T &value) {
        return constant_cell<T>(value);
    }

}  // live_cells

#endif /* LIVE_CELLS_CONSTANT_CELL_HPP */
