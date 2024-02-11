#ifndef LIVE_CELLS_EQUALITY_HPP
#define LIVE_CELLS_EQUALITY_HPP

#include <typeinfo>

#include "computed.hpp"

namespace live_cells {

    namespace internal {

        std::size_t hash_combine(std::size_t seed) {
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
                size_t seed = 0;
                hash_combine(seed, typeid(*this).hash_code(), a.key(), b.key());

                return seed;
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
                size_t seed = 0;
                hash_combine(seed, typeid(*this).hash_code(), a.key(), b.key());

                return seed;
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
    template <typename T, typename U>
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
    template <typename T, typename U>
    auto operator !=(const T &a, const U &b) {
        typedef internal::neq_cell_key<T,U> key;

        return computed(key_ref::create<key>(a, b), a, b, [] (auto a, auto b) {
            return a != b;
        });
    }

}  // live_cells

#endif /* LIVE_CELLS_EQUALITY_HPP */
