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
     * \brief Key identifying a peek_cell.
     */
    struct peek_cell_key : value_key<key_ref> {
        using value_key<key_ref>::value_key;
    };

    /**
     * \brief Maintains the state of a \p peek_cell.
     *
     * This state merely adds an observer on the argument cell when
     * the state is initialized, and removes it when its disposed.
     */
    template <typename C>
    class peek_cell_state : public cell_state, public observer {
    public:
        /**
         * \brief Create a \p peek_cell state.
         *
         * \param k    Key identifying cell
         * \param cell Argument cell
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
        /** \brief Argument cell */
        C cell;
    };

    /**
     * \brief A \p Cell that reads the value of another \p Cell
     * without notifying its observers when it changes.
     */
    template <Cell C>
    class peek_cell : public stateful_cell<peek_cell_state<C>> {
        typedef stateful_cell<peek_cell_state<C>> parent;

    public:
        /**
         * \brief The type of value held by this cell
         */
        typedef C::value_type value_type;

        /**
         * \brief Create a peek cell that reads the value of \a cell.
         *
         * \param cell The cell of which to read the value.
         */
        peek_cell(const C &cell) :
            parent(key_ref::create<peek_cell_key>(cell.key()), cell),
            cell(cell) {}

        /**
         * \brief Get the value of the argument cell.
         *
         * \return The cell value
         */
        typename C::value_type value() const {
            return cell.value();
        }

        /**
         * \brief Get the value of the argument cell and track it as a
         * dependency.
         *
         * \return The cell value
         */
        typename C::value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }

    private:
        /**
         * \brief The argument cell.
         */
        const C cell;
    };

    /**
     * \relates peek_cell
     *
     * \brief Create a \p Cell that reads the value of \a cell but does
     * not notify its observers when the value has changes.
     *
     * \param cell The \p Cell of which to read the value.
     *
     * \return The peek cell
     */
    template <typename C>
    auto peek(const C &cell) {
        return peek_cell<C>(cell);
    }

}  // live_cells

#endif /* LIVE_CELLS_PEEK_CELL_HPP */
