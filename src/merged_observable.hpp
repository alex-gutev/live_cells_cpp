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

#ifndef LIVE_CELLS_MERGED_OBSERVABLE_HPP
#define LIVE_CELLS_MERGED_OBSERVABLE_HPP

namespace live_cells {

    /**
     * A single observable which merges multiple observables.
     *
     * This class allows an observer to be added to multiple
     * observables simultaneously via a single observable object.
     */
    template <Observable T, Observable... Ts>
    class merged_observable {
    public:

        /**
         * Create an observable that merges the observables @a first
         * and @a rest... into one.
         *
         * When add_observer() and remove_observer() are called, they
         * are applied to @a first and every observable in @a rest.
         *
         * @param first An observable
         * @param rest Obseravbles
         */
        merged_observable(T first, Ts... rest) :
            first(first),
            rest(rest...) {}

        void add_observer(observer::ref o) {
            first.add_observer(o);
            rest.add_observer(o);
        }

        void remove_observer(observer::ref o) {
            first.remove_observer(o);
            rest.remove_observer(o);
        }

    private:
        T first;
        merged_observable<Ts...> rest;
    };

    template <Observable T>
    class merged_observable<T> {
    public:

        merged_observable(T o) : first(o) {}

        void add_observer(observer::ref o) {
            first.add_observer(o);
        }

        void remove_observer(observer::ref o) {
            first.remove_observer(o);
        }

    private:
        T first;
    };


}  // live_cells

#endif /* LIVE_CELLS_MERGED_OBSERVABLE_HPP */
