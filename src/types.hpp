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
     * \brief Concept defining an object that can be identified by a
     * key.
     *
     * The following methods are required:
     *
     * - <tt>key_ref key()</tt>
     *
     *   Return the key, which identifies the object.
     */
    template <typename T>
    concept Keyable = requires(const T &o) {
        { o.key() } -> std::same_as<key_ref>;
    };

    /**
     * \brief Concept defining an observable object that can be
     * tracked dynamically as a dependency.
     *
     * The following methods are required:
     *
     * - <tt>operator()</tt>
     *
     *   Return the observable's value and register it as a
     *   dependency.
     */
    template <typename T>
    concept Trackable = requires(const T &o) {
        { o() } -> std::same_as<decltype(o.value())>;
    };

    /**
     * \brief Defines the cell protocol.
     *
     * Cells must define the following types:
     *
     * - \c value_type
     *
     *   The type of value held by the observable.
     *
     * Cell types must the following methods:
     *
     * - <tt>void add_observer(observer::ref o) const</tt>;
     *
     *   Add observer \a o to this cell.
     *
     * - <tt>void remove_observer(observer::ref o) const</tt>;
     *
     *   Remove observer \a o from this cell.
     *
     *   Implementations should only remove \a o after this method is
     *   called the same number of times as \c add_observer() was called
     *   with the same observer \a o.
     *
     * - <tt>value_type value() const</tt>;
     *
     *   Return the value held by the observable.
     *
     * - <tt>value_type operator()() const</tt>;
     *
     *   Track this cell as a dependency and return its value.
     *
     * - <tt>key_ref key() const</tt>;
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
     * \brief Concept that constrains \p Cell to cells holding a
     * specific value type \a T.
     */
    template <typename C, typename T>
    concept TypedCell = Cell<C> && std::convertible_to<typename C::value_type, T>;

    /**
     * \brief Concept defining a \p Cell that can have its value set.
     *
     * The following method must be defined on \a C to satisfy the
     * constraints:
     *
     * - <tt>void value(value_type)</tt>;
     *
     *   Set the value of the cell, and notify the observers of the
     *   cell that its value has changed.
     */
    template <typename C>
    concept MutableCell = Cell<C> && requires(C c) {
        { c.value(std::declval<C::value_type>()) };
    };

    /**
     * \brief Concept that constrains \p MutableCell to cells holding
     * a specific value type \a T.
     */
    template <typename C, typename T>
    concept TypedMutableCell = TypedCell<C,T> && MutableCell<C>;

}  // live_cells

#endif /* LIVE_CELLS_TYPES_HPP */
