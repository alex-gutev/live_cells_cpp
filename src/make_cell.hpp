#ifndef LIVE_CELLS_MAKE_CELL_HPP
#define LIVE_CELLS_MAKE_CELL_HPP

#include <type_traits>

#include "observable.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * The return type of C::value().
     */
    template <typename C>
    using cell_value_type = decltype(std::declval<C>().value());

    /**
     * Define a cell class using `C` such that the constraints defined
     * by the concept `Cell` are satisifed.
     *
     * In-effect, this class can be used to "generate" a cell
     * interface from a "prototype" that does not satisfy all the
     * constraints of `Cell`.
     *
     * IMPORTANT: This class may be inherited but may not be used
     * polymorphically.
     *
     * C::value(), C::add_observer(), C::remove_observer() are used to
     * implement the value(), add_observer() and remove_observer()
     * methods respectively.
     *
     * The return type of C::value() is used for `value_type`.
     *
     * If C::key() exists (that is `C` satisfies the `Keyable`
     * concept), it is used to implement the key() method. Otherwise a
     * default implementation is provided that returns the key given
     * in the constructor.
     *
     * If C::operator() exists (that is `C` satisfies the `Trackable`
     * concept), it is used to implement the operator()
     * method. Otherwise a default implementation is provided.
     */
    template <typename C>
    struct make_cell {
        /**
         * Cell value type (return type of C::value()).
         */
        typedef cell_value_type<C> value_type;

        /**
         * Constructor that allows a `key` to be provided, with `args`
         * forwarded to the constructor of `C`.
         *
         * This is only available if C::key() does not exist.
         */
        template <typename K, typename... Args>
        requires (!Keyable<C> && std::constructible_from<C,Args...>)
        make_cell(K&& key, Args&&... args) :
            cell(std::forward<Args>(args)...),
            key_(std::forward<K>(key)) {}

        /**
         * Constructor that forwards all its arguments `args` to the
         * constructor of `C`.
         *
         * A `unique_key` is used as the key for this cell.
         *
         * This is only available if C::key() does not exist.
         */
        template <typename... Args>
        requires (!Keyable<C> && std::constructible_from<C,Args...>)
        make_cell(Args&&... args) :
            cell(std::forward<Args>(args)...),
            key_(key_ref::create<unique_key>()) {}

        /**
         * Constructor that forwards all its arguments `args` to the
         * constructor of `C`.
         *
         * This constructor does not generate or accept any key
         * arguments. If any are provided, they are forwarded to the
         * constructor of `C`.
         *
         * This is only available if C::key() exists.
         */
        template <typename... Args>
        requires (Keyable<C> && std::constructible_from<C,Args...>)
        make_cell(Args&&... args) :
            cell(std::forward<Args>(args)...) {}

        /** Cell methods */

        void add_observer(observer::ref obs) const {
            cell.add_observer(obs);
        }

        void remove_observer(observer::ref obs) const {
            cell.remove_observer(obs);
        }

        value_type value() const {
            return cell.value();
        }

        value_type operator()() const requires (!Trackable<C>) {
            argument_tracker::global().track_argument(*this);
            return value();
        }

        value_type operator()() const requires (Trackable<C>) {
            return cell();
        }

        key_ref key() const requires Keyable<C> {
            return cell.key();
        }

        key_ref key() const requires (!Keyable<C>) {
            return key_;
        }

    protected:
        C cell;

        const std::enable_if<!Keyable<C>, key_ref>::type key_;
    };

    /**
     * Same as `make_cell` but also provides a value setter `void
     * value(value_type)` for mutable cells.
     */
    template <typename C, typename Parent = make_cell<C>>
    class make_mutable_cell : public Parent {
        using Parent::make_cell;

    public:
        using Parent::value;

        /**
         * Set the value of the cell to `value.
         *
         * The value is forwarded to `C::value(value)`.
         */
        void value(Parent::value_type value) {
            Parent::cell.value(value);
        }
    };

}  // live_cells

#endif /* LIVE_CELLS_MAKE_CELL_HPP */
