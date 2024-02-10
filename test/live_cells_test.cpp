#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE live_cells

#include <string>
#include <memory>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "live_cells.hpp"

using live_cells::use;

/**
 * An observer that records the number of times it's update() method
 * was called.
 */
struct simple_observer : live_cells::observer {
    /**
     * Number of times update() was called.
     */
    size_t notify_count = 0;

    void will_update(const live_cells::key::ref &k) override {}

    void update(const live_cells::key::ref &k) override {
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

    void will_update(const live_cells::key::ref &k) override {}

    void update(const live_cells::key::ref &k) override {
        values.push_back(cell.value());
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


BOOST_AUTO_TEST_SUITE(constant_cell)

BOOST_AUTO_TEST_CASE(constructor_integer) {
    auto cell = live_cells::value_cell(10);

    BOOST_CHECK_EQUAL(cell.value(), 10);
}

BOOST_AUTO_TEST_CASE(constructor_string) {
    auto cell = live_cells::value_cell(std::string("Hello World"));

    BOOST_CHECK_EQUAL(cell.value(), std::string("Hello World"));
}

BOOST_AUTO_TEST_CASE(constructor_bool) {
    auto cell1 = live_cells::value_cell(true);
    auto cell2 = live_cells::value_cell(false);

    BOOST_CHECK_EQUAL(cell1.value(), true);
    BOOST_CHECK_EQUAL(cell2.value(), false);
}

BOOST_AUTO_TEST_CASE(constructor_null) {
    auto cell = live_cells::value_cell<void*>(nullptr);

    BOOST_CHECK_EQUAL(cell.value(), nullptr);
}

// TODO: Add enum test cases

BOOST_AUTO_TEST_CASE(same_values_compare_equal) {
    live_cells::observable_ref cell1(live_cells::value_cell(3));
    live_cells::observable_ref cell2(live_cells::value_cell(3));

    std::hash<live_cells::observable_ref> hash;

    BOOST_CHECK(cell1 == cell2);
    BOOST_CHECK(!(cell1 != cell2));
    BOOST_CHECK(hash(cell1) == hash(cell2));
}

BOOST_AUTO_TEST_CASE(different_value_compare_not_equal) {
    live_cells::observable_ref cell1(live_cells::value_cell(1));
    live_cells::observable_ref cell2(live_cells::value_cell(2));

    BOOST_CHECK(cell1 != cell2);
    BOOST_CHECK(!(cell1 == cell2));
}

// TODO: Add tests for cells wrapped in observable_ref

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(mutable_cell)

BOOST_AUTO_TEST_CASE(keeps_initial_value) {
    auto cell = live_cells::variable(15);
    BOOST_CHECK_EQUAL(cell.value(), 15);
}

BOOST_AUTO_TEST_CASE(set_value_changes_value) {
    auto cell = live_cells::variable(15);
    cell.value(23);

    BOOST_CHECK_EQUAL(cell.value(), 23);
}

BOOST_AUTO_TEST_CASE(keeps_latest_set_value) {
    auto cell = live_cells::variable(15);
    cell.value(23);
    cell.value(101);

    BOOST_CHECK_EQUAL(cell.value(), 101);
}

BOOST_AUTO_TEST_CASE(set_value_calls_observers) {
    auto cell = live_cells::variable(10);
    auto observer = std::make_shared<simple_observer>();

    auto guard = with_observer(cell, observer);

    cell.value(23);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(set_value_twice_calls_observers_twice) {
    auto cell = live_cells::variable(10);
    auto observer = std::make_shared<simple_observer>();

    auto guard = with_observer(cell, observer);

    cell.value(23);
    cell.value(101);

    BOOST_CHECK_EQUAL(observer->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(observer_not_called_after_removal) {
    auto cell = live_cells::variable(10);
    auto observer = std::make_shared<simple_observer>();

    {
        auto guard = with_observer(cell, observer);
        cell.value(23);
    }

    cell.value(101);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observer_not_called_if_new_value_equals_old) {
    auto cell = live_cells::variable(56);
    auto observer = std::make_shared<simple_observer>();

    auto guard = with_observer(cell, observer);
    cell.value(56);

    BOOST_CHECK_EQUAL(observer->notify_count, 0);
}

BOOST_AUTO_TEST_CASE(all_observers_called) {
    auto cell = live_cells::variable(3);

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    auto guard1 = with_observer(cell, observer1);
    cell.value(5);

    auto guard2 = with_observer(cell, observer2);
    cell.value(8);
    cell.value(12);

    BOOST_CHECK_EQUAL(observer1->notify_count, 3);
    BOOST_CHECK_EQUAL(observer2->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(value_updated_when_observer_called) {
    auto cell = live_cells::variable<std::string>("hello");
    auto observer = std::make_shared<value_observer<std::string>>(cell);

    auto guard = with_observer(cell, observer);

    cell.value("bye");

    BOOST_CHECK(observer->values == std::vector<std::string>({"bye"}));
}

BOOST_AUTO_TEST_CASE(batch_updates) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(0);

    auto observer1 = std::make_shared<value_observer<int>>(a);
    auto observer2 = std::make_shared<value_observer<int>>(b);

    auto guard1 = with_observer(a, observer1);
    auto guard2 = with_observer(b, observer2);

    live_cells::batch([&] {
        a.value(1);
        b.value(2);
    });

    BOOST_CHECK(observer1->values == std::vector<int>({1}));
    BOOST_CHECK(observer2->values == std::vector<int>({2}));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(equality_comparison)

BOOST_AUTO_TEST_CASE(constant_cells_equal_if_equal_values) {
    auto a = live_cells::value_cell(1);
    auto b = live_cells::value_cell(1);

    auto eq = a == b;

    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_not_equal_if_unequal_values) {
    auto a = live_cells::value_cell(1);
    auto b = live_cells::value_cell(2);

    auto eq = a == b;

    BOOST_CHECK(!eq.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_unequal_if_unequal_values) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(4);

    auto neq = a != b;

    BOOST_CHECK(neq.value());
}

BOOST_AUTO_TEST_CASE(constant_cells_not_unequal_if_equal_values) {
    auto a = live_cells::value_cell(3);
    auto b = live_cells::value_cell(3);

    auto neq = a != b;

    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_recomputed_when_1st_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    a.value(4);

    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_recomputed_when_2nd_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    b.value(3);

    BOOST_CHECK(eq.value());
}

BOOST_AUTO_TEST_CASE(equality_cell_observers_notified_on_1st_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(eq, observer);

    a.value(4);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(equality_cell_observers_notified_on_2nd_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto eq = a == b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(eq, observer);

    b.value(3);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(inequality_cell_recomputed_when_1st_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    a.value(4);

    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(inequality_cell_recomputed_when_2nd_arg_changes) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    b.value(3);

    BOOST_CHECK(!neq.value());
}

BOOST_AUTO_TEST_CASE(inequality_cell_observers_notified_on_1st_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(neq, observer);

    a.value(4);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(inequality_cell_observers_notified_on_2nd_arg_change) {
    auto a = live_cells::variable(3);
    auto b = live_cells::variable(4);

    auto neq = a != b;

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(neq, observer);

    b.value(3);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}


BOOST_AUTO_TEST_SUITE_END()
