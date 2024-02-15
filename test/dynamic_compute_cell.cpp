#define BOOST_TEST_DYN_LINK

#include <string>
#include <memory>
#include <vector>
#include <exception>

#include <boost/test/unit_test.hpp>

#include "constant_cell.hpp"
#include "mutable_cell.hpp"
#include "computed.hpp"

#include "test_util.hpp"
#include "test_lifecyle.hpp"

BOOST_AUTO_TEST_SUITE(dynamic_compute_cell)

BOOST_AUTO_TEST_CASE(applied_on_constant_cell_value) {
    auto a = live_cells::value_cell(1);
    auto b = live_cells::computed([=] {
        return use(a) + 1;
    });

    BOOST_CHECK_EQUAL(b.value(), 2);
}

BOOST_AUTO_TEST_CASE(reevaluated_when_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed([=] {
        return use(a) + 1;
    });

    a.value(5);

    BOOST_CHECK_EQUAL(b.value(), 6);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_1st_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return use(a) + use(b);
    });

    a.value(5);

    BOOST_CHECK_EQUAL(c.value(), 7);
}

BOOST_AUTO_TEST_CASE(nary_compute_cell_reevaluated_when_2nd_argument_cell_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return use(a) + use(b);
    });

    b.value(8);

    BOOST_CHECK_EQUAL(c.value(), 9);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_1st_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return use(a) + use(b);
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    a.value(8);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_when_2nd_argument_changes) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return use(a) + use(b);
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(c, observer);

    b.value(8);

    BOOST_CHECK_EQUAL(observer->notify_count, 1);
}

BOOST_AUTO_TEST_CASE(observers_notified_on_each_change) {
    auto a = live_cells::variable(1);
    auto b = live_cells::variable(2);

    auto c = live_cells::computed([=] {
        return use(a) + use(b);
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

    auto c = live_cells::computed([=] {
        return use(a) + use(b);
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

    auto c = live_cells::computed([=] {
        return use(a) + use(b);
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

BOOST_AUTO_TEST_CASE(arguments_tracked_correctly_in_conditionals) {
    auto a = live_cells::variable(true);
    auto b = live_cells::variable(2);
    auto c = live_cells::variable(3);

    auto d = live_cells::computed([=] {
        return use(a) ? use(b) : use(c);
    });

    auto observer = std::make_shared<value_observer<int>>(d);
    auto guard = with_observer(d, observer);

    b.value(1);
    a.value(false);
    c.value(10);

    BOOST_CHECK(observer->values == std::vector<int>({1, 3, 10}));
}

BOOST_AUTO_TEST_CASE(dynamic_cell_argument_tracked_correctly) {
    auto a = live_cells::variable(true);
    auto b = live_cells::variable(2);
    auto c = live_cells::variable(3);

    auto d = live_cells::computed([=] {
        return use(a) ? use(b) : use(c);
    });

    auto e = live_cells::variable(0);
    auto f = live_cells::computed([=] {
        return use(d) + use(e);
    });

    auto observer = std::make_shared<value_observer<int>>(f);
    auto guard = with_observer(f, observer);

    b.value(1);
    e.value(10);
    a.value(false);
    c.value(10);

    BOOST_CHECK(observer->values == std::vector<int>({1, 11, 13, 20}));
}

BOOST_AUTO_TEST_CASE(value_preserved_when_none_called) {
    auto a = live_cells::variable(10);
    auto evens = live_cells::computed([=] {
        if ((use(a) % 2) != 0) {
            live_cells::none();
        }

        return use(a);
    });

    auto observer = std::make_shared<value_observer<int>>(evens);
    auto guard = with_observer(evens, observer);

    a.value(1);
    a.value(2);
    a.value(3);
    a.value(4);
    a.value(5);

    BOOST_CHECK(observer->values == std::vector<int>({10, 2, 4}));
}

BOOST_AUTO_TEST_CASE(exception_in_init_handled) {
    auto cell = live_cells::computed([=] {
        throw an_exception();
        return 0;
    });

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(exception_in_init_reproduced_on_access_while_observed) {
    auto cell = live_cells::computed([=] {
        throw an_exception();
        return 0;
    });

    auto observer = std::make_shared<simple_observer>();
    auto guard = with_observer(cell, observer);

    BOOST_CHECK_THROW(cell.value(), an_exception);
}

BOOST_AUTO_TEST_CASE(compares_equal_if_same_key) {
    using live_cells::key_ref;

    typedef value_key<std::string> key_type;

    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::observable_ref c1 = live_cells::computed(key_ref::create<key_type>("the-key"), [=] {
        return use(a) + use(b);
    });

    live_cells::observable_ref c2 = live_cells::computed(key_ref::create<key_type>("the-key"), [=] {
        return use(a) + use(b);
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

    live_cells::observable_ref c1 = live_cells::computed(key_ref::create<key_type>("the-key1"), [=] {
        return use(a) + use(b);
    });

    live_cells::observable_ref c2 = live_cells::computed(key_ref::create<key_type>("the-key2"), [=] {
        return use(a) + use(b);
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_CASE(compares_not_equal_with_default_key) {
    auto a = live_cells::variable(0);
    auto b = live_cells::variable(1);

    live_cells::observable_ref c1 = live_cells::computed([=] {
        return use(a) + use(b);
    });

    live_cells::observable_ref c2 = live_cells::computed([=] {
        return use(a) + use(b);
    });

    BOOST_CHECK(c1 != c2);
    BOOST_CHECK(!(c1 == c2));
}

BOOST_AUTO_TEST_CASE(keyed_cells_manage_same_observers) {
    auto counter = std::make_shared<state_counter>();

    {
        auto key = live_cells::key_ref::create<value_key<std::string>>("key");

        auto a = test_managed_cell<int>(counter, 0);

        auto f = [=] {
            return live_cells::computed(key, [=] {
                return use(a) + 1;
            });
        };

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 0);
        BOOST_CHECK_EQUAL(counter->dispose_count, 0);

        auto observer = std::make_shared<simple_observer>();

        f().add_observer(observer);
        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);

        f().remove_observer(observer);
        BOOST_CHECK_EQUAL(counter->dispose_count, 1);
    }

    BOOST_CHECK_EQUAL(counter->dtor_count, 1);
}

BOOST_AUTO_TEST_CASE(keyed_cell_state_reinitialized_on_add_observer_post_dispose) {
    auto counter = std::make_shared<state_counter>();
    auto key = live_cells::key_ref::create<value_key<std::string>>("key");

    auto a = test_managed_cell<int>(counter, 0);

    auto f = [=] {
        return live_cells::computed(key, [=] {
            return use(a) + 1;
        });
    };

    BOOST_CHECK_EQUAL(counter->ctor_count, 1);
    BOOST_CHECK_EQUAL(counter->init_count, 0);
    BOOST_CHECK_EQUAL(counter->dispose_count, 0);

    auto observer = std::make_shared<simple_observer>();

    {
        auto cell = f();
        auto guard = with_observer(cell, observer);

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 1);
    }

    BOOST_CHECK_EQUAL(counter->dispose_count, 1);

    {
        auto cell = f();
        auto guard = with_observer(cell, observer);

        BOOST_CHECK_EQUAL(counter->ctor_count, 1);
        BOOST_CHECK_EQUAL(counter->init_count, 2);
    }

    BOOST_CHECK_EQUAL(counter->dispose_count, 2);
}

BOOST_AUTO_TEST_SUITE_END()
