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
     * \brief Dynamically typed \p Cell container.
     *
     * This class erases the types of \p Cell's, so that a cell can be
     * used and stored in containers, when its exact type is not known
     * at compile-time.
     */
    class cell {
    public:
        /**
         * \brief Create a container holding the \p Cell \a o.
         *
         * \param o The cell to hold in the container
         */
        template <Cell O>
        cell(O o) :
            obs_ref(std::static_pointer_cast<internal::ref_base>(std::make_shared<internal::typed_ref<O>>(o))) {}

        /**
         * \brief Add an observer to the underlying \p Cell.
         *
         * \param obs The observer to add to the cell.
         */
        void add_observer(observer::ref obs) {
            obs_ref->add_observer(obs);
        }

        /**
         * \brief Remove an observer from the underlying \p Cell.
         *
         * \note The observer is only removed when this method is
         * called the same number of times \ref add_observer was
         * called for the same observer \a obs.
         *
         * \param obs The observer to remove from the cell.
         */
        void remove_observer(observer::ref obs) {
            obs_ref->remove_observer(obs);
        }

        /**
         * \brief Get the key that uniquely identifies the underlying
         * \p Cell.
         *
         * \return The key identifying the underlying cell.
         */
        key_ref key() const {
            return obs_ref->key();
        }

        /**
         * \brief Get the value held by the underlying \p Cell.
         *
         * This method attempts to cast the underlying \p Cell to a cell
         * holding a value of type \a T. If the underlying cell does
         * not hold a value of type \a T, an \p std::bad_cast exception is
         * thrown.
         *
         * \return The value of the underlying cell.
         */
        template <typename T>
        T value() const {
            auto &base = *obs_ref;
            auto &typed = dynamic_cast<internal::typed_ref_base<T>&>(base);

            return typed.value();
        }

        /**
         * \brief Set the value of the underlying \p Cell.
         *
         * This method attempts to cast the underlying cell to a cell
         * holding a value of type \a T. If the underlying cell does
         * not hold a value of type \a T, or is not a \p MutableCell, an
         * \p std::bad_cast exception is thrown.
         *
         * \param value The value to set the underlying cell to.
         */
        template <typename T>
        void value(T value) {
            auto &base = *obs_ref;
            auto &typed = dynamic_cast<internal::mutable_cell_ref<T>&>(base);

            typed.value(value);
        }

    protected:

        /**
         * \brief Pointer to the container holding the underlying
         * cell.
         */
        std::shared_ptr<internal::ref_base> obs_ref;
    };

    /**
     * \brief Dynamically typed \p Cell container with a static value
     * type.
     *
     * Like \p cell, this class erases the types of \p Cell's, so that
     * a cell can be used and stored in containers when its exact type
     * is not known at compile-time. However, this class enforces that
     * the value type of the underlying \p Cell is convertible to \a
     * T.
     */
    template <typename T>
    class typed_cell {
    public:
        /**
         * \brief Create a container holding the \p Cell \a cell.
         *
         * \param cell The cell to hold in the container
         */
        template <TypedCell<T> C>
        typed_cell(C cell) :
            ref(std::static_pointer_cast<internal::typed_ref_base<T>>(std::make_shared<internal::typed_ref<C>>(cell))) {
        }

        /**
         * \brief Add an observer to the underlying \p Cell.
         *
         * \param obs The observer to add to the cell.
         */
        void add_observer(observer::ref obs) {
            ref->add_observer(obs);
        }

        /**
         * \brief Remove an observer from the underlying \p Cell.
         *
         * \note The observer is only removed when this method is
         * called the same number of times \ref add_observer was
         * called for the same observer \a obs.
         *
         * \param obs The observer to remove from the cell.
         */
        void remove_observer(observer::ref obs) {
            ref->remove_observer(obs);
        }

        /**
         * \brief Get the key that uniquely identifies the underlying
         * \p Cell.
         *
         * \return The key identifying the underlying cell.
         */
        key_ref key() const {
            return ref->key();
        }

        /**
         * \brief Get the value held by the underlying \p Cell.
         *
         * \return The value of the underlying cell.
         */
        T value() const {
            return ref->value();
        }

        /**
         * \brief Set the value of the underlying \p Cell.
         *
         * If the underlying cell does is not a \p MutableCell, an \p
         * std::bad_cast exception is thrown.
         *
         * \param value The value to set the underlying cell to.
         */
        void value(T value) {
            auto &base = *ref;
            auto &typed = dynamic_cast<internal::mutable_cell_ref<T>&>(base);

            typed.value(value);
        }

    private:
        /**
         * \brief Pointer to the container holding the underlying
         * cell.
         */
        std::shared_ptr<internal::typed_ref_base<T>> ref;
    };

    /**
     * \relates cell
     *
     * \brief Compare two \p cell's by their keys.
     *
     * \param a A dynamically typed cell
     * \param b A dynamically typed cell
     *
     * \return \p true if the key of \a a is equal to the key of \a b.
     */
    inline bool operator==(const cell &a, const cell &b) {
        return a.key() == b.key();
    }

    /**
     * \relates cell
     *
     * \brief Compare two \p cell's by their keys.
     *
     * \param a A dynamically typed cell
     * \param b A dynamically typed cell
     *
     * \return \p true if the key of \a a is not equal to the key of
     * \a b.
     */
    inline bool operator!=(const cell &a, const cell &b) {
        return !(a == b);
    }


    /**
     * \relates typed_cell
     *
     * \brief Compare two \p typed_cell's by their keys.
     *
     * \param a A dynamically typed cell
     * \param b A dynamically typed cell
     *
     * \return \p true if the key of \a a is equal to the key of \a b.
     */
    template <typename T1, typename T2>
    inline bool operator==(const typed_cell<T1> &a, const typed_cell<T2> &b) {
        return a.key() == b.key();
    }

    /**
     * \relates typed_cell
     *
     * \brief Compare two \p typed_cell's by their keys.
     *
     * \param a A dynamically typed cell
     * \param b A dynamically typed cell
     *
     * \return \p true if the key of \a a is not equal to the key of
     * \a b.
     */
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
