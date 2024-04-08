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

#ifndef LIVE_CELLS_CONSTANT_CELL_HPP
#define LIVE_CELLS_CONSTANT_CELL_HPP

#include <functional>
#include <memory>

#include "observable.hpp"

namespace live_cells {

    /**
     * \brief Key identifying a \p constant_cell.
     */
    template <typename T>
    struct constant_key : key {
        /** \brief Constant value */
        const T value;

        /**
         * \brief Create a constant cell key identifying a \p
         * constant_cell with a given value.
         *
         * \param value The value
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
     * \brief A cell which holds a constant value
     *
     * Adding observers to this cell has no effect since its value
     * never changes.
     */
    template <typename T>
    class constant_cell {
    public:
        /** \brief The type of the constant value. */
        typedef T value_type;

        /**
         * \brief Construct a cell with a constant value.
         *
         * \note Every \p constant_cell holding the same \a value is
         * identified by the same key.
         *
         * \param value The constant value.
         */
        constant_cell(const T &value) : m_value(value) {}

        /**
         * \brief Get the key that identifies this cell.
         *
         * All \p constant_cell's with the same value have the same
         * key.
         *
         * \return The key.
         */
        key_ref key() const {
            return key_ref::create<constant_key<T>>(m_value);
        }

        /**
         * \brief Get the constant value held by this cell.
         *
         * \return The constant value.
         */
        T value() const {
            return m_value;
        }

        void add_observer(observer::ref) const {
            // Do nothing
        }

        void remove_observer(observer::ref) const {
            // Do nothing
        }

        /**
         * \brief Get the constant value held by this cell.
         *
         * \note This cell is not tracked as a dependency since its
         * value never changes.
         *
         * \return The constant value.
         */
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
