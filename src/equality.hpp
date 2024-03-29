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

#ifndef LIVE_CELLS_EQUALITY_HPP
#define LIVE_CELLS_EQUALITY_HPP

#include <typeinfo>

#include "computed.hpp"
#include "util.hpp"

namespace live_cells {

    namespace internal {

        /**
         * Equality comparison cell key
         */
        template <typename T, typename U>
        struct eq_cell_key : key {
            const T a;
            const U b;

            eq_cell_key(T a, U b) : a(a), b(b) {}

            bool eq(const key &k) const noexcept override {
                auto *ptr = dynamic_cast<const eq_cell_key*>(&k);

                return ptr != nullptr &&
                    a.key() == ptr->a.key() &&
                    b.key() == ptr->b.key();
            }

            size_t hash() const noexcept override {
                return hash_combine(0, typeid(*this).hash_code(), a.key(), b.key());
            }
        };

        /**
         * Inequality comparison cell key
         */
        template <typename T, typename U>
        struct neq_cell_key : key {
            const T a;
            const U b;

            neq_cell_key(T a, U b) : a(a), b(b) {}

            bool eq(const key &k) const noexcept override {
                auto *ptr = dynamic_cast<const neq_cell_key*>(&k);

                return ptr != nullptr &&
                    a.key() == ptr->a.key() &&
                    b.key() == ptr->b.key();
            }

            size_t hash() const noexcept override {
                return hash_combine(0, typeid(*this).hash_code(), a.key(), b.key());
            }
        };

    }  // internal

    /**
     * Create a cell which compares two cells for equality by ==.
     *
     * @param a A cell
     * @param b A cell
     *
     * @return A computed cell which evaluates the expression a == b.
     */
    template <Cell T, Cell U>
    auto operator ==(const T &a, const U &b) {
        typedef internal::eq_cell_key<T,U> key;

        return computed(key_ref::create<key>(a, b), a, b, [] (auto a, auto b) {
            return a == b;
        });
    }

    /**
     * Create a cell which compares two cells for inequality by !=.
     *
     * @param a A cell
     * @param b A cell
     *
     * @return A computed cell which evaluates the expression a != b.
     */
    template <Cell T, Cell U>
    auto operator !=(const T &a, const U &b) {
        typedef internal::neq_cell_key<T,U> key;

        return computed(key_ref::create<key>(a, b), a, b, [] (auto a, auto b) {
            return a != b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_EQUALITY_HPP */
