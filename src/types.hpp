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

#ifndef LIVE_CELLS_TYPES_HPP
#define LIVE_CELLS_TYPES_HPP

#include <concepts>

#include "keys.hpp"

namespace live_cells {

    /**
     * \brief Defines the interface for observing changes to the value
     * of a \p Cell.
     */
    class observer {
    public:
        /**
         * \brief Shared pointer to an \p observer.
         */
        typedef std::shared_ptr<observer> ref;

        virtual ~observer() noexcept = default;

        /**
         * \brief Notifies this observer that the value of the \p Cell
         * identified by \a k is going to change.
         *
         * \param k identifying observable
         */
        virtual void will_update(const key_ref &k) = 0;

        /**
         * \brief Notifies this observer that the value of the \p Cell
         * identified by \a k has changed.
         *
         * \param k identifying observable
         */
        virtual void update(const key_ref &k) = 0;
    };

    /**
     * Concept defining an object which can be identified by a key.
     *
     * The following methods are required:
     *
     * - key_ref key()
     *
     *   Return the key, which identifies the object.
     */
    template <typename T>
    concept Keyable = requires(const T &o) {
        { o.key() } -> std::same_as<key_ref>;
    };

    /**
     * Concept defining an observable that can be tracked dynamically
     * as a dependency.
     *
     * The following methods are required:
     *
     * - operator()
     *
     *   Return the observable's value and register it as a
     *   dependency.
     */
    template <typename T>
    concept Trackable = requires(const T &o) {
        { o() } -> std::same_as<decltype(o.value())>;
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
     * - void add_observer(observer::ref o) const;
     *
     *   Add observer `o` for this Cell.
     *
     * - void remove_observer(observer::ref o) const;
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
    concept Cell = requires(const T &o) {
        { o.add_observer(observer::ref()) };
        { o.remove_observer(observer::ref()) };
        { o.value() } -> std::same_as<typename T::value_type>;
        { o() } -> std::same_as<typename T::value_type>;
        { o.key() } -> std::same_as<key_ref>;
    };

    /**
     * Concept defining a Cell with a specific value type `T`.
     */
    template <typename C, typename T>
    concept TypedCell = Cell<C> && std::convertible_to<typename C::value_type, T>;

    /**
     * Concept defining a Cell which can have its value set.
     *
     * The following method must be defined on `C` to satisfy the
     * constraints:
     *
     * - void value(value_type);
     *
     *   Set the value of the cell.
     */
    template <typename C>
    concept MutableCell = Cell<C> && requires(C c) {
        { c.value(std::declval<C::value_type>()) };
    };

    /**
     * Concept defining a MutableCell with a specific value type `T`.
     */
    template <typename C, typename T>
    concept TypedMutableCell = TypedCell<C,T> && MutableCell<C>;

}  // live_cells

#endif /* LIVE_CELLS_TYPES_HPP */
