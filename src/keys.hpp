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

#ifndef LIVE_CELLS_KEYS_HPP
#define LIVE_CELLS_KEYS_HPP

#include <typeinfo>
#include <functional>
#include <memory>

#include "util.hpp"

namespace live_cells {

    /**
     * \brief Defines the interface for a \e key which uniquely
     * identifies a cell.
     */
    class key {
    public:
        virtual ~key() noexcept = default;

        /**
         * \brief Compare this key to another key for equality.
         *
         * \param k The key to compare to
         *
         * \return \p true if this is equal to \aa k.
         */
        virtual bool eq(const key &k) const noexcept = 0;

        /**
         * \brief Compute the hash code for this key.
         *
         * \return Hash code
         */
        virtual std::size_t hash() const noexcept = 0;

        /**
         * \brief Is this a unique key?
         *
         * A key is unique if it is identified by a single \p key
         * instance.
         *
         * \return \p true if this key is unique.
         */
        virtual bool is_unique() const noexcept {
            return false;
        }
    };

    /**
     * \brief Dynamically type \p key container.
     *
     * This container holds a \p key, while preserving its runtime
     * type.
     *
     * Copying this class only copies the reference, not the
     * underlying object, which is kept in memory until the last \p
     * key_ref pointing to it is destroyed.
     */
    class key_ref {
    public:
        /**
         * \brief Create a \p key_ref holding a key of type \a T.
         *
         * A new \a T is constructed in place with arguments \a args
         * passed to the constructor.
         *
         * \param args Arguments to pass to constructor of \a T.
         */
        template <typename T, typename... Args>
        static key_ref create(Args... args) {
            return key_ref(
                std::static_pointer_cast<const key>(
                    std::make_shared<T>(args...)
                )
            );
        }

        /**
         * Get a reference to the underlying \p key.
         *
         * \return Reference to underlying \p key.
         */
        const key &operator *() const {
            return *key_;
        }

        /**
         * Access a member of the underlying \p key.
         *
         * \return Pointer to underlying \p key.
         */
        const key *operator->() const {
            return key_.get();
        }

    private:
        /**
         * \brief Reference to the underlying key.
         */
        std::shared_ptr<const key> key_;

        key_ref(std::shared_ptr<const key> ref) : key_(ref) {}
    };

    /**
     * \brief A key of uniquely identified by a single instance.
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
     * \brief Base class for a key distinguished from other keys by
     * one or more values.
     *
     * Keys of this type compare equal if their runtime type is the
     * same, and their values are equal.
     */
    template <typename T, typename... Ts>
    class value_key : public key {
        /**
         * \brief First value distinguishing this key from other keys.
         */
        const T value;

        /**
         * \brief Key holding remaining values distinguishing they key
         * from other keys.
         */
        const value_key<Ts...> rest;

    public:
        /**
         * \brief Create a key distinguished from other keys by one or
         * more \a values.
         *
         * \param value First value
         * \param rest  Remaining values
         */
        value_key(T value, Ts... rest)
            : value(value),
              rest(rest...) {
        }

        bool eq(const key &other) const noexcept override {
            auto *key = dynamic_cast<const value_key<T,Ts...> *>(&other);

            if (key == nullptr) {
                return false;
            }

            const auto &t1 = typeid(*this);
            const auto &t2 = typeid(other);

            return t1 == t2 && values_equal(*key);
        }

        std::size_t hash() const noexcept override {
            return internal::hash_combine(0, typeid(*this).hash_code(), hash_values());
        }

    private:
        /**
         * \brief Check whether the values of this key are equal to
         * the values of \a key.
         *
         * \param key Another key
         *
         * \return true if the values are equal.
         */
        bool values_equal(const value_key<T,Ts...> key) const noexcept {
            return value == key.value && rest.values_equal(key.rest);
        }

        /**
         * Compute the hash code of the values held in this key.
         *
         * \return The hash code.
         */
        std::size_t hash_values() const noexcept {
            return internal::hash_combine(0, value, rest.hash_values());
        }
    };

    /**
     * \brief Base class for a key distinguished from other keys by one
     * value.
     *
     * Keys of this type compare equal if their runtime type is the
     * same, and their values are equal.
     */
    template <typename T>
    class value_key<T> : public key {
        /**
         * \brief Value distinguishing this key from other keys.
         */
        const T value;

    public:
        /**
         * \brief Create a key distinguished from other keys by one value.
         *
         * \param value The value
         */
        value_key(T value) :
            value(value) {}

        bool eq(const key &other) const noexcept override {
            auto *key = dynamic_cast<const value_key<T> *>(&other);

            if (key == nullptr) {
                return false;
            }

            const auto &t1 = typeid(*this);
            const auto &t2 = typeid(other);

            return t1 == t2 && values_equal(*key);
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

        template <typename T1, typename... Ts>
        friend class value_key;
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
