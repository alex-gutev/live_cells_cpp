/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LIVE_CELLS_MERGED_OBSERVABLE_HPP
#define LIVE_CELLS_MERGED_OBSERVABLE_HPP

namespace live_cells {

    /**
     * \brief Allows multiple cells to be observed at once.
     *
     * This class allows an observer to be added to multiple cells
     * simultaneously via a single call to \p add_observer() and \p
     * remove_observer()
     */
    template <Cell T, Cell... Ts>
    class merged_observable {
    public:

        /**
         * \brief Create a \p merged_observable that adds observers to the cells \a first
         * and \a rest... into.
         *
         * When \p add_observer() and \p remove_observer() are called,
         * they are applied to \a first and every cell in \a rest.
         *
         * \param first A cell
         * \param rest Remaining cells
         */
        merged_observable(T first, Ts... rest) :
            first(first),
            rest(rest...) {}

        /**
         * Add an observer to every cell given in constructor.
         *
         * \param o The observer to add
         */
        void add_observer(observer::ref o) const {
            first.add_observer(o);
            rest.add_observer(o);
        }

        /**
         * Remove an observer from every cell given in constructor.
         *
         * \param o The observer to remove
         */
        void remove_observer(observer::ref o) const {
            first.remove_observer(o);
            rest.remove_observer(o);
        }

    private:
        T first;
        merged_observable<Ts...> rest;
    };

    template <Cell T>
    class merged_observable<T> {
    public:

        merged_observable(T o) : first(o) {}

        void add_observer(observer::ref o) const {
            first.add_observer(o);
        }

        void remove_observer(observer::ref o) const {
            first.remove_observer(o);
        }

    private:
        T first;
    };


}  // live_cells

#endif /* LIVE_CELLS_MERGED_OBSERVABLE_HPP */
