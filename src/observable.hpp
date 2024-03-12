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

    template <typename T>
    class cell;

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
     * Observable types must the following methods:
     *
     * - void add_observer(observer::ref o);
     *
     *   Add observer `o` for this Observable.
     *
     * - void remove_observer(observer::ref o):
     *
     *   Remove observer `o` from the this Observable.
     *
     *   Implementations should only remove `o` after this method is
     *   called the same number of times as add_observer was called
     *   with the same observer `o`.
     *
     * - key_ref key() const;
     *
     *   Return a key that uniquely identifies the observable.
     */
    template <typename T>
    concept Observable = requires(T o) {
        { o.add_observer(observer::ref()) };
        { o.remove_observer(observer::ref()) };
        { o.key() } -> std::same_as<key_ref>;
    };


    /**
     * Polymorphic observable holder.
     *
     * This class erases the types of Observables, so that an
     * observable can be used and stored in containers, when its exact
     * type is not known at compile-time.
     */
    class observable_ref {
    public:
        /**
         * Create an `observable_ref` that wraps the `Observable` `o`.
         */
        template <Observable O>
        observable_ref(O o) :
            obs_ref(std::static_pointer_cast<ref_base>(std::make_shared<typed_ref<O>>(o))) {}

        /**
         * Add an observer to the underlying Observable.
         */
        void add_observer(observer::ref obs) {
            obs_ref->add_observer(obs);
        }

        /**
         * Remove an observer from the underlying Observable.
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
         * Observable.
         */
        key_ref key() const {
            return obs_ref->key();
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
         * Wrapper holding an observable of type O.
         */
        template <Observable O>
        struct typed_ref : ref_base {
            O observable;

            typed_ref(O obs) : observable(obs) {}

            virtual void add_observer(observer::ref obs) {
                observable.add_observer(obs);
            }

            virtual void remove_observer(observer::ref obs) {
                observable.remove_observer(obs);
            }

            virtual key_ref key() const {
                return observable.key();
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
