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

BOOST_AUTO_TEST_CASE(equality_cells_compare_equal_if_same_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::observable_ref eq1(a == b);
    live_cells::observable_ref eq2(a == b);

    std::hash<live_cells::observable_ref> hash;

    BOOST_CHECK(eq1 == eq2);
    BOOST_CHECK(!(eq1 != eq2));
    BOOST_CHECK(hash(eq1) == hash(eq2));
}

BOOST_AUTO_TEST_CASE(equality_cells_compare_not_equal_if_different_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::observable_ref eq1(a == b);
    live_cells::observable_ref eq2(a == live_cells::value_cell(2));
    live_cells::observable_ref eq3(live_cells::value_cell(2) == b);

    BOOST_CHECK(eq1 != eq2);
    BOOST_CHECK(eq1 != eq3);
    BOOST_CHECK(!(eq1 == eq2));
}

BOOST_AUTO_TEST_CASE(inequality_cells_compare_equal_if_same_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::observable_ref neq1(a != b);
    live_cells::observable_ref neq2(a != b);

    std::hash<live_cells::observable_ref> hash;

    BOOST_CHECK(neq1 == neq2);
    BOOST_CHECK(!(neq1 != neq2));
    BOOST_CHECK(hash(neq1) == hash(neq2));
}

BOOST_AUTO_TEST_CASE(inequality_cells_compare_not_equal_if_different_arguments) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    live_cells::observable_ref neq1(a != b);
    live_cells::observable_ref neq2(a != live_cells::value_cell(2));
    live_cells::observable_ref neq3(live_cells::value_cell(2) != b);

    BOOST_CHECK(neq1 != neq2);
    BOOST_CHECK(neq1 != neq3);
    BOOST_CHECK(!(neq1 == neq2));
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(compute_cell)

BOOST_AUTO_TEST_CASE(compute_cell_applied_on_constant_cell_value) {
    auto a = live_cells::value_cell(1);
    auto b = live_cells::computed(a, [] (auto a) {
        return a + 1;
    });

    BOOST_CHECK_EQUAL(b.value(), 2);
}

BOOST_AUTO_TEST_CASE(compute_cell_reevaluated_when_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed(a, [] (auto a) {
        return a + 1;
    });

    a.value(5);

    BOOST_CHECK_EQUAL(b.value(), 6);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_1st_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    a.value(5);

    BOOST_CHECK_EQUAL(c.value(), 7);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_2nd_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    b.value(8);

    BOOST_CHECK_EQUAL(c.value(), 9);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_1st_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    a.value(8);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_2nd_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    b.value(8);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_on_each_change) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    b.value(8);
    a.value(10);
    b.value(100);

    BOOST_CHECK_EQUAL(observer->notify_count, 3);
}

BOOST_AUTO_TEST_CASE(observers_not_called_after_removal) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto observer = std::make_shared<simple_observer>();

    {
        auto guard = with_observer(c, observer);

        b.value(8);
    }

    a.value(10);
    b.value(100);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(all_observers_called) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    auto observer1 = std::make_shared<simple_observer>();
    auto observer2 = std::make_shared<simple_observer>();

    auto guard1 = with_observer(c, observer1);

    b.value(8);

    auto guard2 = with_observer(c, observer2);

    a.value(10);
    b.value(100);

    BOOST_CHECK_EQUAL(observer1->notify_count, 3);
    BOOST_CHECK_EQUAL(observer2->notify_count, 2);
}

BOOST_AUTO_TEST_CASE(compares_equal_if_same_key) {
    using live_cells::key_ref;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::observable_ref c1 = live_cells::computed(key_ref::create<key_type>("the-key"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    live_cells::observable_ref c2 = live_cells::computed(key_ref::create<key_type>("the-key"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    std::hash<live_cells::observable_ref> hash;

    BOOST_CHECK(c1 == c2);
    BOOST_CHECK(!(c1 != c2));
    BOOST_CHECK(hash(c1) == hash(c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_if_same_key) {
    using live_cells::key_ref;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::observable_ref c1 = live_cells::computed(key_ref::create<key_type>("the-key1"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    live_cells::observable_ref c2 = live_cells::computed(key_ref::create<key_type>("the-key2"), a, b, [] (auto a, auto b) {
        return a + b;
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_with_default_key) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::observable_ref c1 = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    live_cells::observable_ref c2 = live_cells::computed(a, b, [] (auto a, auto b) {
        return a + b;
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_SUITE_END()
