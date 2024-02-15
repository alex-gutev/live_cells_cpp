#define BOOST_TEST_DYN_LINK

#include <string>
#include <memory>
#include <vector>
#include <exception>

#include <boost/test/unit_test.hpp>

#include "constant_cell.hpp"
#include "test_util.hpp"

using live_cells::use;


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

BOOST_AUTO_TEST_SUITE_END()
