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
