#ifndef LIVE_CELLS_KEYS_HPP
#define LIVE_CELLS_KEYS_HPP

#include <functional>
#include <memory>

namespace live_cells {

    /**
     * Defines the interface for a 'key' which uniquely identifies a
     * cell.
     */
    class key {
    public:
        typedef std::shared_ptr<const key> ref;

        virtual ~key() noexcept = default;

        /**
         * Compare this key to another key for equality.
         *
         * @param k The key to compare to
         *
         * @return true if this is equal to @a k.
         */
        virtual bool eq(const key &k) const noexcept = 0;

        /**
         * Return a hash code for this key.
         */
        virtual std::size_t hash() const noexcept = 0;

        virtual bool is_unique() const noexcept {
            return false;
        }
    };

    /**
     * A key of which every instance is unique.
     *
     * An instance of this class compares equal only when compared to
     * the same instance.
     */
    class unique_key : public key {
    public:
        static key::ref make_unique() {
            return std::static_pointer_cast<key>(
                std::make_shared<unique_key>()
            );
        }

        bool eq(const key &k) const noexcept override;

        std::size_t hash() const noexcept override;

        bool is_unique() const noexcept override {
            return true;
        }
    };

    inline bool operator ==(const key &k1, const key &k2) {
        return k1.eq(k2);
    }

    inline bool operator !=(const key &k1, const key &k2) {
        return !k1.eq(k2);
    }

    /**
     * Cell key equality comparison function
     */
    struct key_equality {
        bool operator()(const key::ref &k1, const key::ref &k2) const {
            return k1 != nullptr && k2 != nullptr && *k1 == *k2;
        }
    };

    /**
     * Cell key hash function
     */
    struct key_hash {
        std::size_t operator()(const key::ref k) const {
            return k->hash();
        }
    };

}  // live_cells

template<>
struct std::hash<live_cells::key> {
    std::size_t operator()(const live_cells::key& k) const noexcept {
        return k.hash();
    }
};

#endif /* LIVE_CELLS_KEYS_HPP */
