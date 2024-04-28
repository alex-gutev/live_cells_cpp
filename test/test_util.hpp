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

#ifndef TEST_LIVE_CELLS_TEST_UTIL_HPP
#define TEST_LIVE_CELLS_TEST_UTIL_HPP

#include <initializer_list>
#include <cassert>

#include "observable.hpp"

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

    void update(const live_cells::key_ref &k, bool changed) override {
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
    const live_cells::cell cell;

    /**
     * The values of the cell for every update() call.
     */
    std::vector<T> values;

    /**
     * Create a value_observer.
     *
     * @param cell The cell being observed.
     */
    value_observer(const live_cells::cell &cell) :
        cell(cell) {}

    void will_update(const live_cells::key_ref &k) override {
        if (!updating) {
            assert(notify_count == 0);

            updating = true;
            notify_count = 0;
            has_changed = false;
        }

        notify_count++;
    }

    void update(const live_cells::key_ref &k, bool changed) override {
        if (updating) {
            assert(notify_count > 0);

            has_changed = has_changed || changed;

            if (--notify_count == 0) {
                updating = false;

                if (has_changed) {
                    // TODO: Surround with try-catch
                    auto value = cell.value<T>();

                    if (values.empty() || values.back() != value) {
                        values.push_back(value);
                    }
                }
            }
        }
    }

    /**
     * Check that the values received are equal to `vs`.
     */
    template <typename U>
    void check_values(std::initializer_list<U> vs) {
        BOOST_CHECK_EQUAL(values.size(), vs.size());

        auto it = values.begin();

        for (const auto &v : vs) {
            if (it == values.end())
                break;

            BOOST_CHECK_EQUAL(*it, v);
            ++it;
        }
    }

private:
    bool updating = false;
    int notify_count = 0;
    bool has_changed = false;
};

/**
 * Manages the adding and removing of a cell observer.
 */
template <live_cells::Cell O, typename T>
class observer_guard {
    /**
     * The observed cell
     */
    O &cell;

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
    observer_guard(O &cell, std::shared_ptr<T> observer) :
        cell(cell),
        observer(observer) {
        cell.add_observer(observer);
    }

    observer_guard(const observer_guard &) = delete;
    observer_guard(observer_guard&& g) :
        cell(g.cell) {
        observer.swap(g.observer);
    }

    ~observer_guard() {
        if (observer) {
            cell.remove_observer(observer);
        }
    }

    observer_guard &operator ==(const observer_guard &) = delete;
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
template <live_cells::Cell O, typename T>
auto with_observer(O &cell, std::shared_ptr<T> observer) {
    return observer_guard<O,T>(cell, observer);
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
