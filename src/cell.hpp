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

#ifndef LIVE_CELLS_CELL_HPP
#define LIVE_CELLS_CELL_HPP

#include <memory>

#include "keys.hpp"
#include "observable.hpp"

namespace live_cells {

    /**
     * Concept defining a cell.
     *
     * A cell is an Observable which holds a value of type `T`, that
     * is returned by the `value()` method.
     *
     * The following methods should be implemented by Cell types.
     *
     * - T value() const
     *
     *   Return the value held by the cell.
     */
    template <typename C, typename T>
    concept Cell = Observable<C> && requires(C c) {
        { c.value() } -> std::convertible_to<T>;
    };

    /**
     * Polymorphic cell holder, for cells holding values of type `T`.
     *
     * This class erases the types of Cells, so that a Cell can be
     * used and stored in containers, when its exact type is not known
     * at compile-time.
     */
    template <typename T>
    class cell_ref {
    public:
        /**
         * Create a polymorphic holder for @a observable.
         *
         * @param observable The observable.
         */
        template <Cell<T> C>
        cell_ref(C o) :
            ref(std::static_pointer_cast<ref_base>(std::make_shared<typed_ref<C>>(o))) {}

        /**
         * Add an observer to the underlying Cell.
         *
         * See Observable::add_observer.
         */
        void add_observer(observer::ref obs) {
            ref->add_observer(obs);
        }

        /**
         * Remove an observer from the underlying Cell.
         *
         * See Observable::remove_observer.
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
         * Retrieve the value held by the underlying Cell.
         */
        T value() {
            return ref->value();
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

            virtual T value() const = 0;
        };

        /**
         * Wrapper holding a Cell of type C.
         */
        template <Cell<T> C>
        struct typed_ref : ref_base {
            C cell;

            typed_ref(C cell) : cell(cell) {
            }

            virtual void add_observer(observer::ref obs) {
                cell.add_observer(obs);
            }

            virtual void remove_observer(observer::ref obs) {
                cell.remove_observer(obs);
            }

            virtual key_ref key() const {
                return cell.key();
            }

            virtual T value() const {
                return cell.value();
            }
        };

        /**
         * Pointer to the wrapper holding the cell.
         */
        std::shared_ptr<ref_base> ref;
    };

    /**
     * Compares to polymorphic cells by their keys.
     *
     * @param a A polymorphic cell
     * @param b A polymorphic cell
     *
     * @return true if the key of @a a is equal to the key of @a b.
     */
    template <typename A, typename B>
    inline bool operator==(const cell_ref<A> &a, const cell_ref<B> &b) {
        return a.key() == b.key();
    }

    template <typename A, typename B>
    inline bool operator!=(const cell_ref<A> &a, const cell_ref<B> &b) {
        return !(a == b);
    }

}  // live_cells

template<typename T>
struct std::hash<live_cells::cell_ref<T>> {
    std::size_t operator()(const live_cells::cell_ref<T> &a) const noexcept {
        return a.key()->hash();
    }
};

#endif /* LIVE_CELLS_CELL_HPP */
