#define BOOST_TEST_DYN_LINK

#include <string>
#include <memory>
#include <vector>
#include <exception>

#include <boost/test/unit_test.hpp>

#include "live_cells.hpp"
#include "test_util.hpp"


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