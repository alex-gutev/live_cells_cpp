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

#ifndef LIVE_CELLS_PEEK_CELL_HPP
#define LIVE_CELLS_PEEK_CELL_HPP

#include "keys.hpp"
#include "cell_state.hpp"
#include "stateful_cell.hpp"

namespace live_cells {
    /**
     * Key identifying a peek_cell.
     */
    struct peek_cell_key : value_key<key_ref> {
        using value_key<key_ref>::value_key;
    };

    /**
     * Peek cell state.
     *
     * This state merely adds an observer on the argument cell when
     * the state is initialized, and removes it when its disposed.
     */
    template <typename C>
    class peek_cell_state : public cell_state, public observer {
    public:
        /**
         * Create a peek_cell state.
         *
         * @param k    Key identifying cell
         * @param cell Argument cell
         */
        peek_cell_state(key_ref k, C cell) :
            cell_state(k),
            cell(cell) {}

        void will_update(const key_ref &k) override {}
        void update(const key_ref &k) override {}

    protected:
        void init() override {
            cell_state::init();
            cell.add_observer(std::dynamic_pointer_cast<observer>(this->shared_from_this()));
        }

        void pause() override {
            cell.remove_observer(std::dynamic_pointer_cast<observer>(this->shared_from_this()));
            cell_state::pause();
        }

    private:
        /** Argument cell */
        C cell;
    };

    /**
     * Create a peek_cell.
     *
     * A peek cell, has the same value as another cell but it does not
     * notify observers when its value has changed.
     */
    template <Observable C>
    class peek_cell : public stateful_cell<peek_cell_state<C>> {
        typedef stateful_cell<peek_cell_state<C>> parent;

    public:
        typedef C::value_type value_type;

        /**
         * Create a peek cell with the same value as `cell`.
         */
        peek_cell(const C &cell) :
            parent(key_ref::create<peek_cell_key>(cell.key()), cell),
            cell(cell) {}

        typename C::value_type value() const {
            return cell.value();
        }

        typename C::value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }

    private:
        const C cell;
    };

    /**
     * Return a cell with the same value as `cell` but one which does
     * not notify its observer when its value has changed.
     */
    template <typename C>
    auto peek(const C &cell) {
        return peek_cell<C>(cell);
    }

}  // live_cells

#endif /* LIVE_CELLS_PEEK_CELL_HPP */
