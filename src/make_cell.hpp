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

#ifndef LIVE_CELLS_MAKE_CELL_HPP
#define LIVE_CELLS_MAKE_CELL_HPP

#include <type_traits>

#include "observable.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * \brief The return type of C::value().
     */
    template <typename C>
    using cell_value_type = decltype(std::declval<C>().value());

    /**
     * \brief Defines a cell class using \a C such that the
     * constraints defined by the concept \p Cell are satisifed.
     *
     * In-effect, this class \e generates a cell interface from a \e
     * prototype that does not satisfy all the constraints of \p Cell.
     *
     * \warning This class may be inherited but may not be used
     * polymorphically.
     *
     * \p C::value(), \p C::add_observer(), \p C::remove_observer()
     * are used to implement the \p value(), \p add_observer() and \p
     * remove_observer() methods respectively.
     *
     * The return type of \p C::value() is used to define the \p value_type.
     *
     * If \p C::key() exists (that is \a C satisfies the \p Keyable
     * concept), it is used to implement the \p key()
     * method. Otherwise a default implementation is provided that
     * returns the key given in the constructor.
     *
     * If \p C::operator()() exists (that is \a C satisfies the \p
     * Trackable concept), it is used to implement the \p operator()()
     * method. Otherwise a default implementation is provided.
     */
    template <typename C>
    struct make_cell {
        /**
         * \brief Cell value type (return type of \p C::value()).
         */
        typedef cell_value_type<C> value_type;

        /**
         * \brief Constructor that allows a \a key to be provided,
         * with \a args forwarded to the constructor of \a C.
         *
         * \note This is only available if \p C::key() is not defined.
         *
         * \param key The key identifying the cell
         * \param args Arguments forwarded to constructor of \a C
         */
        template <typename K, typename... Args>
        requires (!Keyable<C> && std::constructible_from<C,Args...>)
        make_cell(K&& key, Args&&... args) :
            cell(std::forward<Args>(args)...),
            key_(std::forward<K>(key)) {}

        /**
         * \brief Constructor that forwards all its arguments \a args
         * to the constructor of \a C.
         *
         * A \p unique_key is used as the key for this cell.
         *
         * \note This is only available if \p C::key() is not defined.
         *
         * \param args Arguments forwarded to constructor of \a C
         */
        template <typename... Args>
        requires (!Keyable<C> && std::constructible_from<C,Args...>)
        make_cell(Args&&... args) :
            cell(std::forward<Args>(args)...),
            key_(key_ref::create<unique_key>()) {}

        /**
         * \brief Constructor that forwards all its arguments \a args
         * to the constructor of \a C.
         *
         * This constructor does not generate or accept any key
         * arguments. If any are provided, they are forwarded to the
         * constructor of \a C.
         *
         * \note This is only available if \p C::key() is defined.
         *
         * \param args Arguments forwarded to constructor of \a C
         */
        template <typename... Args>
        requires (Keyable<C> && std::constructible_from<C,Args...>)
        make_cell(Args&&... args) :
            cell(std::forward<Args>(args)...) {}

        /** Cell methods */

        /**
         * \brief Add an observer to the cell.
         *
         * \note Implemented using \p C::add_observer().
         *
         * \param obs Observer to add to cell
         */
        void add_observer(observer::ref obs) const {
            cell.add_observer(obs);
        }

        /**
         * \brief Remove an observer from the cell.
         *
         * \note Implemented using \p C::remove_observer().
         *
         * \param obs Observer to remove from cell
         */
        void remove_observer(observer::ref obs) const {
            cell.remove_observer(obs);
        }

        /**
         * \brief Get the value of the cell.
         *
         * \note Implemented using \p C::value()
         *
         * \return The cell's value
         */
        value_type value() const {
            return cell.value();
        }

        /**
         * \brief Get the value of the cell and track it as a
         * dependency.
         *
         * \note Implemented using \p C::value()
         *
         * \return The cell's value
         */
        value_type operator()() const requires (!Trackable<C>) {
            argument_tracker::global().track_argument(*this);
            return value();
        }

        /**
         * \brief Get the value of the cell and track it as a
         * dependency.
         *
         * \note This is only available if \p C::operator()() is
         * defined.
         *
         * \note Implemented using \p C::operator()()
         *
         * \return The cell's value
         */
        value_type operator()() const requires (Trackable<C>) {
            return cell();
        }

        /**
         * \brief Get the key identifying the cell.
         *
         * \note Implemented using \p C::key().
         *
         * \return The key
         */
        key_ref key() const requires Keyable<C> {
            return cell.key();
        }

        /**
         * \brief Get the key identifying the cell.
         *
         * \note This is only available if \p C::key() is not defined.
         *
         * \return The key
         */
        key_ref key() const requires (!Keyable<C>) {
            return key_;
        }

    protected:
        /**
         * \p The partial cell implementation.
         */
        C cell;

        /**
         * \brief Key identifying the cell.
         *
         * \note This member exists only if \p C::key() is not
         * defined.
         */
        const std::enable_if<!Keyable<C>, key_ref>::type key_;
    };

    /**
     * \brief Same as \p make_cell but also provides a value setter \p
     * value(Parent::value_type) so that the definition satisfies the
     * \p MutableCell concept.
     */
    template <typename C, typename Parent = make_cell<C>>
    class make_mutable_cell : public Parent {
        using Parent::make_cell;

    public:
        using Parent::value;

        /**
         * \brief Set the value of the cell to \a value.
         *
         * \note Implemented using \p C::value(value).
         *
         * \param value The new value of the cell
         *
         * \return \a value
         */
        Parent::value_type operator=(const Parent::value_type &value) {
            this->value(value);
            return value;
        }

        /**
         * \brief Set the value of the cell.
         *
         * This is equivalent to \prop value(value).
         *
         * \note This \c const version is provided to allow setting
         * the value of a mutable cell within a lambda provided for
         * the reverse computation function of a mutable computed
         * cell.
         *
         * \param value The new value
         *
         * \return \a value
         */
        Parent::value_type operator=(const Parent::value_type &value) const {
            this->value(value);
            return value;
        }

        /**
         * \brief Set the value of the cell to \a value.
         *
         * \note Implemented using \p C::value(value).
         *
         * \param value The new value of the cell
         */
        void value(Parent::value_type value) {
            Parent::cell.value(value);
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_MAKE_CELL_HPP */
