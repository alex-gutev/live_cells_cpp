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
