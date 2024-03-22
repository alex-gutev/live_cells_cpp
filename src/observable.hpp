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
#include "types.hpp"

namespace live_cells {

    namespace internal {

        /**
         * Typeless cell wrapper interface.
         */
        struct ref_base {
            virtual ~ref_base() = default;

            virtual void add_observer(observer::ref obs) = 0;
            virtual void remove_observer(observer::ref obs) = 0;

            virtual key_ref key() const = 0;
        };

        /**
         * Base wrapper interface for cells holding values of type T.
         */
        template <typename T>
        struct typed_ref_base : ref_base {
            virtual T value() const = 0;
        };

        /**
         * Base wrapper interface for mutable cells with value type `T`.
         */
        template <typename T>
        struct mutable_cell_ref {
            virtual ~mutable_cell_ref() = default;
            virtual void value(T value) = 0;
        };

        /**
         * Wrapper holding a Cell of type O.
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

            typename O::value_type value() const override {
                return observable.value();
            }
        };

        template <MutableCell C>
        struct typed_ref<C> : typed_ref_base<typename C::value_type>, mutable_cell_ref<typename C::value_type> {
            C observable;

            typed_ref(C obs) : observable(obs) {}

            void add_observer(observer::ref obs) override {
                observable.add_observer(obs);
            }

            void remove_observer(observer::ref obs) override {
                observable.remove_observer(obs);
            }

            key_ref key() const override {
                return observable.key();
            }

            typename C::value_type value() const override {
                return observable.value();
            }

            void value(typename C::value_type value) override {
                observable.value();
            }
        };

    }  // internal

    /**
     * Polymorphic cell holder.
     *
     * This class erases the types of Cells, so that a cell can be
     * used and stored in containers, when its exact type is not known
     * at compile-time.
     */
    class cell {
    public:
        /**
         * Create a `cell` that wraps the `Cell` `o`.
         */
        template <Cell O>
        cell(O o) :
            obs_ref(std::static_pointer_cast<internal::ref_base>(std::make_shared<internal::typed_ref<O>>(o))) {}

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
         * Retrieve the value held by the cell.
         *
         * This method attempts to cast the underlying cell to a cell
         * holding a value of type `T`. If the underlying cell does
         * not hold a value of type `T`, an std::bad_cast exception is
         * thrown.
         */
        template <typename T>
        T value() const {
            auto &base = *obs_ref;
            auto &typed = dynamic_cast<internal::typed_ref_base<T>&>(base);

            return typed.value();
        }

        /**
         * Set the value of the cell to `value`.
         *
         * This method attempts to cast the underlying cell to a cell
         * holding a value of type `T`. If the underlying cell does
         * not hold a value of type `T`, or is not a MutableCell, an
         * std::bad_cast exception is thrown.
         */
        template <typename T>
        void value(T value) {
            auto &base = *obs_ref;
            auto &typed = dynamic_cast<internal::mutable_cell_ref<T>&>(base);

            typed.value(value);
        }

    protected:

        /**
         * Pointer to the wrapper holding the observable.
         */
        std::shared_ptr<internal::ref_base> obs_ref;
    };

    /**
     * Typed polymorphic cell holder.
     *
     * Like `cell`, this class erases the types of Cells, so that a
     * cell can be used and stored in containers, when its exact type
     * is not known at compile-time. However, this class enforces that
     * the value type of the underlying cell is convertible to `T`
     */
    template <typename T>
    class typed_cell {
    public:

        template <TypedCell<T> C>
        typed_cell(C cell) :
            ref(std::static_pointer_cast<internal::typed_ref_base<T>>(std::make_shared<internal::typed_ref<C>>(cell))) {
        }

        /**
         * Add an observer to the underlying Cell.
         */
        void add_observer(observer::ref obs) {
            ref->add_observer(obs);
        }

        /**
         * Remove an observer from the underlying Cell.
         *
         * NOTE: The observer is only removed when this method is
         * called the same number of times `add_observer(obs)` was
         * called for the same observer `obs`.
         */
        void remove_observer(observer::ref obs) {
            ref->remove_observer(obs);
        }

        /**
         * Returns the key that uniquely identifies the underlying
         * Cell.
         */
        key_ref key() const {
            return ref->key();
        }

        /**
         * Retrieve the value held by the cell.
         *
         * This method attempts to cast the underlying cell to a cell
         * holding a value of type `T`. If the underlying cell does
         * not hold a value of type `T`, an std::bad_cast exception is
         * thrown.
         */
        T value() const {
            return ref->value();
        }

        /**
         * Set the value of the cell to `value`.
         *
         * This method attempts to cast the underlying cell to a cell
         * holding a value of type `T`. If the underlying cell does
         * not hold a value of type `T`, or is not a MutableCell, an
         * std::bad_cast exception is thrown.
         */
        void value(T value) {
            auto &base = *ref;
            auto &typed = dynamic_cast<internal::mutable_cell_ref<T>&>(base);

            typed.value(value);
        }

    private:
        std::shared_ptr<internal::typed_ref_base<T>> ref;
    };

    /**
     * Compare two polymorphic cells by their keys.
     *
     * @param a A polymorphic observable
     * @param b A polymorphic observable
     *
     * @return true if the key of @a a is equal to the key of @a b.
     */
    inline bool operator==(const cell &a, const cell &b) {
        return a.key() == b.key();
    }

    inline bool operator!=(const cell &a, const cell &b) {
        return !(a == b);
    }


    /**
     * Compare two typed polymorphic cells by their keys.
     *
     * @param a A polymorphic observable
     * @param b A polymorphic observable
     *
     * @return true if the key of @a a is equal to the key of @a b.
     */
    template <typename T1, typename T2>
    inline bool operator==(const typed_cell<T1> &a, const typed_cell<T2> &b) {
        return a.key() == b.key();
    }

    template <typename T1, typename T2>
    inline bool operator!=(const typed_cell<T1> &a, const typed_cell<T2> &b) {
        return !(a == b);
    }

}  // live_cells


template<>
struct std::hash<live_cells::cell> {
    std::size_t operator()(const live_cells::cell &a) const noexcept {
        return a.key()->hash();
    }
};

template<typename T>
struct std::hash<live_cells::typed_cell<T>> {
    std::size_t operator()(const live_cells::typed_cell<T> &a) const noexcept {
        return a.key()->hash();
    }
};

#endif /* LIVE_CELLS_OBSERVABLE_H */
