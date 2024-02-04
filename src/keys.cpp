#include "keys.hpp"

bool live_cells::unique_key::eq(const key &k) const noexcept {
    return this == &k;
}

std::size_t live_cells::unique_key::hash() const noexcept {
    std::hash<const unique_key*> hash;

    return hash(this);
}
