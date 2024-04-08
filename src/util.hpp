/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LIVE_CELLS_UTIL_HPP
#define LIVE_CELLS_UTIL_HPP

#include <tuple>
#include <utility>

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

        /**
         * Count the number of variadic template parameters.
         *
         * The `size` member variable contains the number of
         * parameters in `Ts`.
         */
        template <typename... Ts>
        struct count_args {
        };

        template <typename T, typename... Ts>
        struct count_args<T, Ts...> {
            static constexpr size_t size = 1 + count_args<Ts...>::size;
        };

        template <>
        struct count_args<> {
            static constexpr size_t size = 0;
        };

        /**
         * Check that the variadic template parameter list `Ts` has
         * `S` parameters.
         *
         * The `value` member is true if the condition holds.
         */
        template <size_t S, typename... Ts>
        struct is_size {
            static constexpr bool value = S == count_args<Ts...>::size;
        };

        /**
         * Check that the variadic template parameter list `Ts` has at
         * least `S` parameters.
         *
         * The `value` member is true if the condition holds.
         */
        template <size_t S, typename... Ts>
        struct is_at_least {
            static constexpr bool value = count_args<Ts...>::size >= S;
        };

        /**
         * Helper class for packing a variadic argument list into an
         * std::tuple.
         *
         * `S` is the size of the parameter list to stop packing
         * at.
         */
        template <size_t S, bool enable, typename... As>
        struct packer {
        };

        template <size_t S, typename A, typename... As>
        struct packer<S, false, A, As...> {
            template <typename Tuple>
            static auto pack(const Tuple &packed, const A &arg, const As&... args) {
                return packer<S, is_size<S, As...>::value, As...>::pack(std::tuple_cat(packed, std::make_tuple(arg)), args...);
            }
        };

        template <size_t S, typename... As>
        struct packer<S, true, As...> {
            template <typename Tuple>
            static auto pack(const Tuple &packed, const As&... args) {
                return std::make_tuple(packed, args...);
            }
        };

        /**
         * Pack a variable argument list into an std::tuple.
         *
         * @param args Variable argument list.
         *
         * @return A tuple where the first element is a tuple
         *    containing the packed argument list, and the remaining
         *    elements are the last `S` elements in the argument list.
         */
        template <size_t S, typename... As>
        auto pack(const As&... args) {
            static_assert(is_at_least<S, As...>::value, "Not enough arguments provided to pack()");

            return packer<S, is_size<S, As...>::value, As...>::pack(std::make_tuple(), args...);
        }

        /**
         * Apply a function `fn` on an argument list unpacked from a
         * tuple.
         *
         * @param fn   The function to apply
         * @param args Tuple of arguments
         *
         * @return Return value of fn(...).
         */
        template <typename F, typename Tuple, std::size_t... I>
        auto unpack(const F &fn, const Tuple &args, std::index_sequence<I...>) {
            return fn(std::get<I>(args)...);
        }

        template <typename F, typename Tuple>
        auto unpack(const F &fn, const Tuple &args) {
            constexpr auto size = std::tuple_size<Tuple>::value;
            return unpack(fn, args, std::make_index_sequence<size>());
        }

    }  // internal

}  // live_cells

#endif /* LIVE_CELLS_UTIL_HPP */
