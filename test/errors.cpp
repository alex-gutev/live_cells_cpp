#define BOOST_TEST_DYN_LINK

#include <string>
#include <memory>
#include <vector>
#include <exception>

#include <boost/test/unit_test.hpp>

#include "constant_cell.hpp"
#include "mutable_cell.hpp"
#include "errors.hpp"

#include "test_util.hpp"


BOOST_AUTO_TEST_SUITE(error_handling)

BOOST_AUTO_TEST_CASE(on_error_catch_all) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed([=] () {
        if (use(a) <= 0) {
            throw an_exception();
        }

        return use(a);
    });

    auto c = live_cells::variable(2);
    auto result = live_cells::on_error(b, c);

    auto observer = std::make_shared<value_observer<int>>(result);
    auto guard = with_observer(result, observer);

    BOOST_CHECK_EQUAL(result.value(), 1);

    a.value(0);
    c.value(4);
    a.value(10);
    c.value(100);

    BOOST_CHECK(observer->values == std::vector<int>({2, 4, 10}));
}

BOOST_AUTO_TEST_CASE(on_error_with_exception_type) {
    auto a = live_cells::variable(1);
    auto b = live_cells::computed([=] () {
        if (use(a) < 0) {
            throw an_exception();
        }
        else if (use(a) == 0) {
            throw std::invalid_argument("A cannot be 0");
        }

        return use(a);
    });

    auto c = live_cells::variable(2);
    auto result = live_cells::on_error<std::invalid_argument>(b, c);

    {
        auto observer = std::make_shared<value_observer<int>>(result);
        auto guard = with_observer(result, observer);

        BOOST_CHECK_EQUAL(result.value(), 1);

        a.value(0);
        c.value(4);
        a.value(10);
        c.value(100);

        BOOST_CHECK(observer->values == std::vector<int>({2, 4, 10}));
    }

    a.value(-1);
    BOOST_CHECK_THROW(result.value(), an_exception);
}

BOOST_AUTO_TEST_SUITE_END()
