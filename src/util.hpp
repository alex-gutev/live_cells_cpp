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

#ifndef LIVE_CELLS_UTIL_HPP
#define LIVE_CELLS_UTIL_HPP

namespace live_cells {

    namespace internal {

        inline std::size_t hash_combine(std::size_t seed) {
            return seed;
        }

        /**
         * Combine the hash code (using std::hash) of @a v and @a rest.
         *
         * @param seed Starting hash value
         * @param v    A value
         * @param rest Remaining values
         *
         * @return Combined hash code
         */
        template <typename T, typename... Rest>
        std::size_t hash_combine(std::size_t seed, const T& v, const Rest&... rest)
        {
            seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return hash_combine(seed, rest...);
        }

    }  // internal

}  // live_cells

#endif /* LIVE_CELLS_UTIL_HPP */
