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

#ifndef LIVE_CELLS_OBSERVABLE_H
#define LIVE_CELLS_OBSERVABLE_H

#include <concepts>

#include "keys.hpp"

namespace live_cells {

    /**
     * Defines the observer interface for observing a cell.
     */
    class observer {
    public:
        /**
         * observer pointer type
         */
        typedef std::shared_ptr<observer> ref;

        virtual ~observer() noexcept = default;

        /**
         * Notifies this observer that the value of the observable
         * identified by @a k is going to change.
         *
         * @param Key identifying observable
         */
        virtual void will_update(const key_ref &k) = 0;

        /**
         * Notifies this observer that the value of the observable
         * identified by @a k has changed.
         *
         * @param Key identifying observable
         */
        virtual void update(const key_ref &k) = 0;
    };

    /**
     * Concept defining an observable object.
     *
     * Cells must define the following types:
     *
     * - value_type
     *
     *   The type of value held by the observable.
     *
     * Cell types must the following methods:
     *
     * - void add_observer(observer::ref o);
     *
     *   Add observer `o` for this Cell.
     *
     * - void remove_observer(observer::ref o):
     *
     *   Remove observer `o` from this Cell.
     *
     *   Implementations should only remove `o` after this method is
     *   called the same number of times as add_observer was called
     *   with the same observer `o`.
     *
     * - value_type value() const;
     *
     *   Return the value held by the observable.
     *
     * - value_type operator()() const;
     *
     *   Track this cell as a referenced argument cell and return its
     *   value.
     *
     * - key_ref key() const;
     *
     *   Return a key that uniquely identifies the observable.
     */
    template <typename T>
    concept Cell = requires(T o) {
        { o.add_observer(observer::ref()) };
        { o.remove_observer(observer::ref()) };
        { o.value() } -> std::same_as<typename T::value_type>;
        { o() } -> std::same_as<typename T::value_type>;
        { o.key() } -> std::same_as<key_ref>;
    };


    /**
     * Polymorphic observable holder.
     *
     * This class erases the types of Cells, so that a cell can be
     * used and stored in containers, when its exact type is not known
     * at compile-time.
     */
    class observable_ref {
    public:
        /**
         * Create an `observable_ref` that wraps the `Cell` `o`.
         */
        template <Cell O>
        observable_ref(O o) :
            obs_ref(std::static_pointer_cast<ref_base>(std::make_shared<typed_ref<O>>(o))) {}

        /**
         * Add an observer to the underlying Cell.
         */
        void add_observer(observer::ref obs) {
            obs_ref->add_observer(obs);
        }

        /**
         * Remove an observer from the underlying Cell.
         *
         * NOTE: The observer is only removed when this method is
         * called the same number of times `add_observer(obs)` was
         * called for the same observer `obs`.
         */
        void remove_observer(observer::ref obs) {
            obs_ref->remove_observer(obs);
        }

        /**
         * Returns the key that uniquely identifies the underlying
         * Cell.
         */
        key_ref key() const {
            return obs_ref->key();
        }

        /**
         * Retrieve the value held by the observable.
         *
         * This method attempts to cast the underlying observable to
         * an observable holding a value of type `T`. If the
         * underlying observable does not hold a value of type `T` an
         * std::bad_cast exception is thrown.
         */
        template <typename T>
        T value() const {
            auto &base = *obs_ref;
            auto &typed = dynamic_cast<typed_ref_base<T>&>(base);

            return typed.value();
        }

    private:
        /**
         * Typeless wrapper interface.
         */
        struct ref_base {
            virtual ~ref_base() = default;

            virtual void add_observer(observer::ref obs) = 0;
            virtual void remove_observer(observer::ref obs) = 0;

            virtual key_ref key() const = 0;
        };

        /**
         * Base wrapper interface for observables holding values of
         * type T.
         */
        template <typename T>
        struct typed_ref_base : ref_base {
            virtual T value() const = 0;
        };

        /**
         * Wrapper holding an observable of type O.
         */
        template <Cell O>
        struct typed_ref : typed_ref_base<typename O::value_type> {
            O observable;

            typed_ref(O obs) : observable(obs) {}

            void add_observer(observer::ref obs) override {
                observable.add_observer(obs);
            }

            void remove_observer(observer::ref obs) override {
                observable.remove_observer(obs);
            }

            key_ref key() const override {
                return observable.key();
            }

            O::value_type value() const override {
                return observable.value();
            }
        };

        /**
         * Pointer to the wrapper holding the observable.
         */
        std::shared_ptr<ref_base> obs_ref;
    };

    /**
     * Compare two polymorphic observables by their keys.
     *
     * @param a A polymorphic observable
     * @param b A polymorphic observable
     *
     * @return true if the key of @a a is equal to the key of @a b.
     */
    inline bool operator==(const observable_ref &a, const observable_ref &b) {
        return a.key() == b.key();
    }

    inline bool operator!=(const observable_ref &a, const observable_ref &b) {
        return !(a == b);
    }

}  // live_cells


template<>
struct std::hash<live_cells::observable_ref> {
    std::size_t operator()(const live_cells::observable_ref &a) const noexcept {
        return a.key()->hash();
    }
};

#endif /* LIVE_CELLS_OBSERVABLE_H */
