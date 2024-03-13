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

#ifndef TEST_LIVE_CELLS_TEST_LIFECYCLE_HPP
#define TEST_LIVE_CELLS_TEST_LIFECYCLE_HPP

#include <memory>

#include "stateful_cell.hpp"

/**
 * Tracks the number of times each cell state lifecycle method was
 * called.
 */
struct state_counter {
    /**
     * Number of times the constructor was called.
     */
    size_t ctor_count = 0;

    /**
     * Number of times the destructor was called.
     */
    size_t dtor_count = 0;

    /**
     * Number of times init() was called.
     */
    size_t init_count = 0;

    /**
     * Number of times dispose() was called.
     */
    size_t dispose_count = 0;
};

/**
 * Cell state for testing the cell lifecycle.
 */
class test_managed_cell_state : public live_cells::cell_state {
    /**
     * Lifecycle methods call count tracker.
     */
    std::shared_ptr<state_counter> counter;

public:
    /**
     * Create the cell state for testing the cell lifecycle.
     *
     * This state increments the method call counts, in @a counter,
     * whenever the constructor, destructor, init() and dispose() are
     * called.
     *
     * @param key Key identifying cell
     *
     * @param counter Lifecycle method call counter
     */
    test_managed_cell_state(live_cells::key_ref key, std::shared_ptr<state_counter> counter) :
        cell_state(key),
        counter(counter) {
        counter->ctor_count++;
    }

    ~test_managed_cell_state() {
        counter->dtor_count++;
    }

    void init() override {
        counter->init_count++;
        live_cells::cell_state::init();
    }

    void pause() override {
        counter->dispose_count++;
        live_cells::cell_state::pause();
    }
};

/**
 * Cell state for testing the cell lifecycle.
 */
template <typename T>
class test_managed_cell : public live_cells::stateful_cell<test_managed_cell_state> {
    /** Shorthand for parent class */
    typedef live_cells::stateful_cell<test_managed_cell_state> parent;

    /** Constant cell value */
    const T value_;

public:
    /**
     * Create a cell for testing the cell lifecycle.
     *
     * The cell's state object increments the method call counts, in
     * @a counter, whenever the state constructor, destructor, init()
     * and dispose() are called.
     *
     * @param counter Lifecycle method call counter
     *
     * @param value   The cell's (constant) value.
     */
    test_managed_cell(std::shared_ptr<state_counter> counter, T value) :
        parent(live_cells::key_ref::create<live_cells::unique_key>(), counter),
        value_(value) {}

    test_managed_cell(const test_managed_cell &cell) :
        parent(cell),
        value_(cell.value_) {
    }

    T value() const {
        return value_;
    }

    T operator()() const {
        live_cells::argument_tracker::global().track_argument(*this);
        return value();
    }
};

#endif /* TEST_LIVE_CELLS_TEST_LIFECYCLE_HPP */
