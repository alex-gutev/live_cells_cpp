#ifndef LIVE_CELLS_KEYS_HPP
#define LIVE_CELLS_KEYS_HPP

#include <functional>
#include <memory>

#include "util.hpp"

namespace live_cells {

    /**
     * Defines the interface for a 'key' which uniquely identifies a
     * cell.
     */
    class key {
    public:
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
     * Polymorphic key holder.
     *
     * This class holds a key, while preserving its runtime type.
     *
     * Copying this class only copies the reference, not the
     * underlying object, which is kept in memory until the last
     * key_ref pointing to it is destroyed.
     */
    class key_ref {
    public:
        /**
         * Create a key_ref holding a key of type @a T.
         *
         * A new @a T is constructed with arguments @a args.
         *
         * @param args Arguments to pass to constructor of @a T.
         */
        template <typename T, typename... Args>
        static key_ref create(Args... args) {
            return key_ref(
                std::static_pointer_cast<const key>(
                    std::make_shared<T>(args...)
                )
            );
        }

        const key &operator *() const {
            return *key_;
        }

        const key *operator->() const {
            return key_.get();
        }

    private:
        /**
         * Reference to the underlying key.
         */
        std::shared_ptr<const key> key_;

        key_ref(std::shared_ptr<const key> ref) : key_(ref) {}
    };

    /**
     * A key of which every instance is unique.
     *
     * An instance of this class compares equal only when compared to
     * the same instance.
     */
    class unique_key : public key {
    public:
        bool eq(const key &k) const noexcept override;

        std::size_t hash() const noexcept override;

        bool is_unique() const noexcept override {
            return true;
        }
    };

    /**
     * Base class for a key distinguished from other keys by one or
     * more values.
     *
     * Keys of this type compare equal if their runtime type is the
     * same, and their values are equal.
     */
    template <typename T, typename... Ts>
    class value_key : public key {
        /**
         * First value distinguishing this key from other keys.
         */
        const T value;

        /**
         * Remaining values distinguishing this key from other keys.
         */
        const value_key<Ts...> rest;

    public:
        /**
         * Create a key distinguished from other keys by one or more
         * values.
         *
         * @param value First value
         * @param rest  Remaining values
         */
        value_key(T value, Ts... rest)
            : value(value),
              rest(rest...) {
        }

        bool eq(const key &other) const noexcept override {
            auto *key = dynamic_cast<const value_key<T,Ts...> *>(other);

            return key != nullptr && values_equal(*key);
        }

        std::size_t hash() const noexcept override {
            return internal::hash_combine(0, typeid(*this).hash_code(), hash_values());
        }

    private:
        /**
         * Check whether the values of this key are equal to the
         * values of @a key.
         *
         * @param key Another key
         *
         * @return true if the values are equal.
         */
        bool values_equal(const value_key<T,Ts...> key) const noexcept {
            return value == key.value && rest.values_equal(key.rest);
        }

        /**
         * Compute the hash code of the values held in this key.
         *
         * @return The combined hash code.
         */
        std::size_t hash_values() const noexcept {
            return internal::hash_combine(0, value, rest);
        }
    };

    /**
     * Base class for a key distinguished from other keys by one
     * value.
     *
     * Keys of this type compare equal if their runtime type is the
     * same, and their values are equal.
     */
    template <typename T>
    class value_key<T> : public key {
        /**
         * Value distinguishing this key from other keys.
         */
        const T value;

    public:
        /**
         * Create a key distinguished from other keys by one value.
         *
         * @param value The value
         */
        value_key(T value) :
            value(value) {}

        bool eq(const key &other) const noexcept override {
            auto *key = dynamic_cast<const value_key<T> *>(&other);

            return key != nullptr && values_equal(*key);
        }

        std::size_t hash() const noexcept override {
            return internal::hash_combine(0, typeid(*this).hash_code(), hash_values());
        }

    private:
        bool values_equal(const value_key<T> key) const noexcept {
            return value == key.value;
        }

        std::size_t hash_values() const noexcept {
            return std::hash<T>{}(value);
        }

    };

    inline bool operator ==(const key &k1, const key &k2) {
        return k1.eq(k2);
    }

    inline bool operator !=(const key &k1, const key &k2) {
        return !k1.eq(k2);
    }

    inline bool operator ==(const key_ref &k1, const key_ref &k2) {
        return *k1 == *k2;
    }

    inline bool operator !=(const key_ref &k1, const key_ref &k2) {
        return *k1 != *k2;
    }

}  // live_cells

template<>
struct std::hash<live_cells::key> {
    std::size_t operator()(const live_cells::key& k) const noexcept {
        return k.hash();
    }
};

template<>
struct std::hash<live_cells::key_ref> {
    std::size_t operator()(const live_cells::key_ref &k) const noexcept {
        return k->hash();
    }
};

#endif /* LIVE_CELLS_KEYS_HPP */
