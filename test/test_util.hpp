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

#ifndef TEST_LIVE_CELLS_TEST_UTIL_HPP
#define TEST_LIVE_CELLS_TEST_UTIL_HPP

/**
 * An observer that records the number of times it's update() method
 * was called.
 */
struct simple_observer : live_cells::observer {
    /**
     * Number of times update() was called.
     */
    size_t notify_count = 0;

    void will_update(const live_cells::key_ref &k) override {}

    void update(const live_cells::key_ref &k) override {
        notify_count++;
    }
};

/**
 * An observer that records the values of the observed cell, at the
 * time update() was called.
 */
template <typename T>
struct value_observer : live_cells::observer {
    /**
     * The observed cell
     */
    const live_cells::cell<T> &cell;

    /**
     * The values of the cell for every update() call.
     */
    std::vector<T> values;

    /**
     * Create a value_observer.
     *
     * @param cell The cell being observed.
     */
    value_observer(const live_cells::cell<T> &cell) :
        cell(cell) {}

    void will_update(const live_cells::key_ref &k) override {}

    void update(const live_cells::key_ref &k) override {
        auto value = cell.value();

        if (values.empty() || values.back() != value) {
            values.push_back(value);
        }
    }
};

/**
 * Manages the adding and removing of a cell observer.
 */
template <typename T>
class observer_guard {
    /**
     * The observed cell
     */
    live_cells::observable &cell;

    /**
     * The observer
     */
    std::shared_ptr<T> observer;

public:
    /**
     * Add observer @a observer to the observers of @a cell.
     *
     * The observer is removed when this object is destroyed.
     *
     * @param cell     The cell to observe
     * @param observer The observer
     */
    observer_guard(live_cells::observable &cell, std::shared_ptr<T> observer) :
        cell(cell),
        observer(observer) {
        cell.add_observer(observer);
    }

    observer_guard(const observer_guard<T> &) = delete;
    observer_guard(observer_guard<T>&& g) :
        cell(g.cell) {
        observer.swap(g.observer);
    }

    ~observer_guard() {
        if (observer) {
            cell.remove_observer(observer);
        }
    }

    observer_guard &operator ==(const observer_guard<T> &) = delete;
};

/**
 * Add an observer to a cell.
 *
 * @param cell     The cell
 * @param observer The observer
 *
 * @return An observer_guard that removes the observer when the guard
 *   object is destroyed.
 */
template <typename T>
observer_guard<T> with_observer(live_cells::observable &cell, std::shared_ptr<T> observer) {
    return observer_guard<T>(cell, observer);
}

template <typename T>
class value_key : public live_cells::key {
    const T value;

public:
    value_key(T&& value) :
        value(std::forward<T>(value)) {}

    bool eq(const live_cells::key &k) const noexcept override {
        auto *k2 = dynamic_cast<const value_key<T>*>(&k);

        return k2 != nullptr && value == k2->value;
    }

    std::size_t hash() const noexcept override {
        std::hash<T> hash;

        return hash(value);
    }
};

/**
 * Exception class used to test exception handling
 */
struct an_exception : std::exception {};

#endif /* TEST_LIVE_CELLS_TEST_UTIL_HPP */
