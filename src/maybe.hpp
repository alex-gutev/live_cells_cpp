#ifndef LIVE_CELLS_MAYBE_HPP
#define LIVE_CELLS_MAYBE_HPP

#include <concepts>
#include <exception>
#include <optional>

#include "util.hpp"
#include "exceptions.hpp"

namespace live_cells {
    /**
     * \brief Container holding a cell value or an exception that
     * occurred while computing a value.
     */
    template <typename T>
    class maybe {
    public:
        /**
         * \brief Create a container holding an \p
         * uninitialized_cell_error exception.
         */
        maybe() {
            try {
                throw uninitialized_cell_error();
            }
            catch (...) {
                error_ = std::current_exception();
            }
        }

        /**
         * \brief Create a container holding a value.
         *
         * \param val The value
         */
        maybe(const T &val) : value_(val) {}

        /**
         * \brief Create a container holding an exception that was
         * thrown.
         *
         * \param error The exception that was thrown.
         */
        maybe(std::exception_ptr error) : error_(error) {}

        /**
         * \brief Create a container holding an exception that was
         * thrown.
         *
         * \note This should be used instead of the constructor when
         * the type of the exception is known and the exception is not
         * wrapped in an \c std::exception_ptr.
         *
         * \param error The exception that was thrown.
         *
         * \return A \p maybe holding an exception.
         */
        static maybe<T> error(const auto &error) {
            try {
                throw error;
            }
            catch (...) {
                return maybe(std::current_exception());
            }
        }

        /**
         * \brief Create a \p maybe holding the result of calling \a
         * f.
         *
         * \a f is called with no arguments. If it returns normally,
         * the resulting \p maybe holds the returned value. If \a f
         * throws an exception the resulting maybe holds the thrown
         * exception.
         *
         * \param A function of no arguments.
         *
         * \return A \p maybe holding the result of calling \a f.
         */
        static maybe<T> wrap(const std::invocable auto &f) {
            try {
                return maybe(f());
            }
            catch (...) {
                return maybe(std::current_exception());
            }
        }

        /**
         * \brief Get the value or throw the exception stored in this
         * container.
         *
         * \return The value stored in this container.
         */
        T unwrap() const {
            if (value_) {
                return value_.value();
            }

            std::rethrow_exception(error_);
        }

        /**
         * \brief Get the value held in this.
         *
         * \return The value held in this container.
         *
         * \throws std::bad_optional_access If the maybe does not hold
         * a value.
         */
        std::optional<T> value() const {
            return value_;
        }

        /**
         * \brief Get the exception held in this maybe.
         *
         * \return The exception held in this maybe.
         */
        std::exception_ptr error() const {
            return error_;
        }

    private:
        /**
         * \brief The value held in this container.
         */
        std::optional<T> value_;

        /**
         * \brief The error held in this container.
         */
        std::exception_ptr error_;
    };

    /**
     * \brief Create a \p maybe holding the result of calling \a
     * f.
     *
     * \a f is called with no arguments. If it returns normally,
     * the resulting \p maybe holds the returned value. If \a f
     * throws an exception the resulting maybe holds the thrown
     * exception.
     *
     * \param A function of no arguments.
     *
     * \return A \p maybe holding the result of calling \a f.
     */
    auto maybe_wrap(const std::invocable auto &f) {
        return maybe<decltype(f())>::wrap(f);
    }

    template <typename A, typename B>
    bool operator==(const maybe<A> &a, const maybe<B> &b) {
        return a.value() == b.value() && a.error() == b.error();
    }

}  // live_cells

template<typename T>
struct std::hash<live_cells::maybe<T>> {
    std::size_t operator()(const live_cells::maybe<T> &m) {
        return live_cells::internal::hash_combine(0, m.value(), m.error());
    }
};

#endif /* LIVE_CELLS_MAYBE_HPP */
