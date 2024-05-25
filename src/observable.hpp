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
            virtual T operator()() const = 0;
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

            typename O::value_type operator()() const override {
                return observable();
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
                observable.value(value);
            }

            typename C::value_type operator()() const override {
                return observable();
            }
        };

    }  // internal

    template <typename T>
    class typed_cell;

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
         * \brief Create a container holding the \p Cell held in \a c.
         *
         * \param c A \p typed_cell container.
         */
        template <typename T>
        cell(typed_cell<T> c) :
            obs_ref(std::static_pointer_cast<internal::ref_base>(c.ref)) {
        }

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

        /**
         * \brief Get the value held by the underlying \p Cell and
         * track it as a dependency.
         *
         * This method attempts to cast the underlying \p Cell to a
         * cell holding a value of type \a T. If the underlying cell
         * does not hold a value of type \a T, an \p std::bad_cast
         * exception is thrown.
         *
         * \return The value of the underlying cell.
         */
        template <typename T>
        T operator()() const {
            auto &base = *obs_ref;
            auto &typed = dynamic_cast<internal::typed_ref_base<T>&>(base);

            return typed();
        }

    protected:

        /**
         * \brief Pointer to the container holding the underlying
         * cell.
         */
        std::shared_ptr<internal::ref_base> obs_ref;

        template <typename T>
        friend class typed_cell;
    };

    /**
     * \brief Exception thrown when attempting to cast a \p cell to a
     * \p typed_cell with a value type that is incompatible with the
     * cell's value type.
     */
    struct bad_typed_cell_cast : std::exception {
        const char *what() const noexcept override {
            return "Attempt to cast a `cell` to a `typed_cell<>` with a value type"
                " that is incompatible with the cell's value type.";
        }
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
         * \brief Create a container holding the \p Cell held in \a c.
         *
         * \param c A typeless \p cell container.
         *
         * \throws bad_typed_cell_cast When the value type of the \p
         *    Cell held in \a c is not compatible with the value type
         *    \p T of this container.
         */
        typed_cell(cell c) :
            ref(std::dynamic_pointer_cast<internal::typed_ref_base<T>>(c.obs_ref)) {

            if (!ref) {
                throw bad_typed_cell_cast();
            }
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

        /**
         * \brief Get the value held by the underlying \p Cell and
         * track it as a dependency.
         *
         * \return The value of the underlying cell.
         */
        T operator()() const {
            return ref();
        }

    private:
        /**
         * \brief Pointer to the container holding the underlying
         * cell.
         */
        std::shared_ptr<internal::typed_ref_base<T>> ref;

        friend class cell;
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
