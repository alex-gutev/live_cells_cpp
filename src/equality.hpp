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

#include "keys.hpp"
#include "computed.hpp"
#include "util.hpp"

namespace live_cells {

    namespace internal {

        /**
         * \brief Equality comparison cell key
         */
        struct eq_cell_key : value_key<key_ref,key_ref> {
            using value_key<key_ref,key_ref>::value_key;
        };

        /**
         * \brief Inequality comparison cell key
         */
        struct neq_cell_key : value_key<key_ref,key_ref> {
            using value_key<key_ref,key_ref>::value_key;
        };

    }  // internal

    /**
     * \brief Create a \p Cell that compares two cells for equality by
     * \p ==.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A computed cell which evaluates the expression \a a == \a b.
     */
    template <typename T, typename U>
    auto operator ==(const T &a, const U &b) requires (
        (Cell<T> && CellOrValue<U>) ||
        (CellOrValue<T> && Cell<U>)
    ) {
        typedef internal::eq_cell_key key;

        auto cell_a = ensure_cell(a);
        auto cell_b = ensure_cell(b);

        return computed(key_ref::create<key>(cell_a.key(), cell_b.key()), cell_a, cell_b, [] (auto a, auto b) {
            return a == b;
        });
    }

    /**
     * \brief Create a \p Cell that compares two cells for inequality
     * by \p !=.
     *
     * \param a A \p Cell or value
     * \param b A \p Cell or value
     *
     * \return A computed cell that evaluates the expression \a a != \a b.
     */
    template <typename T, typename U>
    auto operator !=(const T &a, const U &b) requires (
        (Cell<T> && CellOrValue<U>) ||
        (CellOrValue<T> && Cell<U>)
    ) {
        typedef internal::neq_cell_key key;

        auto cell_a = ensure_cell(a);
        auto cell_b = ensure_cell(b);

        return computed(key_ref::create<key>(cell_a.key(), cell_b.key()), cell_a, cell_b, [] (auto a, auto b) {
            return a != b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_EQUALITY_HPP */
