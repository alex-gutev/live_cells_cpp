#ifndef LIVE_CELLS_PREVIOUS_VALUE_CELL_HPP
#define LIVE_CELLS_PREVIOUS_VALUE_CELL_HPP

#include "keys.hpp"
#include "stateful_cell.hpp"
#include "cell_state.hpp"
#include "observer_cell_state.hpp"
#include "maybe.hpp"
#include "exceptions.hpp"

namespace live_cells {

    /**
     * \brief Key identifying a \p previous_value_cell.
     */
    template <typename T>
    struct previous_value_cell_key : value_key<T> {
        using value_key<T>::value_key;
    };

    /**
     * \brief Maintains the state of a \p previous_value_cell.
     */
    template <Cell C>
    class previous_value_cell_state : public cell_state, public observer, public observer_cell_state {
        /**
         * \brief The cell's value type
         */
        typedef typename C::value_type value_type;

    public:
        /**
         * \brief Create a \p previous_value_cell state.
         *
         * \param k    Key identifying the cell
         * \param cell The argument cell
         */
        previous_value_cell_state(key_ref k, C cell) :
            cell_state(k),
            cell(cell),

            prev_value(maybe<value_type>::error(uninitialized_cell_error())),

            current_value(maybe<value_type>::wrap([cell] {
                return cell.value();
            })) {

            stale = false;
        }

        /**
         * \brief Get the stored previous value.
         *
         * \return The previous value
         */
        value_type value() {
            if (stale) {
                update_current_value();
            }

            return prev_value.unwrap();
        }

    protected:
        void init() override {
            cell_state::init();
            init_observer_state();

            cell.add_observer(observer_ptr());

            current_value = maybe<value_type>::wrap([this] {
                return cell.value();
            });
        }

        void pause() override {
            cell.remove_observer(observer_ptr());

            cell_state::pause();
            pause_observer_state();
        }

        void will_update(const key_ref &k) override {
            handle_will_update([this] {
                notify_will_update();
            });
        }

        void update(const key_ref &k, bool changed) override {
            handle_update(
                changed,
                [] { return true; },

                [this] (bool changed) {
                    if (stale) {
                        update_current_value();
                    }

                    notify_update(changed);
                },

                [] {}
            );
        }

    private:
        /**
         * \brief The argument cell.
         */
        const C cell;

        /**
         * \brief The previous value of the cell.
         */
        maybe<value_type> prev_value;

        /**
         * \brief The current value of the cell.
         */
        maybe<value_type> current_value;

        /**
         * \brief Get an \p observer::ref for this, that can be passed to
         * \p add_observer and \p remove_observer of \p Cell.
         */
        std::shared_ptr<observer> observer_ptr() {
            return std::dynamic_pointer_cast<observer>(this->shared_from_this());
        }

        /**
         * \brief Update the stored previous and current cell value.
         *
         * \p prev_value is set to \p current_value.
         * \p current_value is set to the current value of \p cell.
         */
        void update_current_value() {
            auto next_value = maybe<value_type>::wrap([this] {
                return cell.value();
            });

            if (current_value == next_value) {
                stale = false;
                return;
            }

            prev_value = current_value;
            current_value = next_value;

            stale = false;
        }
    };

    /**
     * \brief A \p Cell that evaluates to the previous value of
     * another \p Cell.
     *
     * \attention This cell requires at least one observer to
     * function.
     */
    template <Cell C>
    class previous_value_cell : public stateful_cell<previous_value_cell_state<C>> {

        /**
         * \brief Shorthand for the parent class.
         */
        typedef stateful_cell<previous_value_cell_state<C>> parent;

        /**
         * \brief Shorthand for cell key type.
         */
        typedef previous_value_cell_key<key_ref> key_type;

    public:
        /**
         * \brief The cell's value type.
         */
        typedef typename C::value_type value_type;

        /**
         * \brief Create a cell that evaluates to the previous value
         * of \a cell.
         *
         * \param cell The argument cell.
         */
        previous_value_cell(const C &cell) :
            parent(key_ref::create<key_type>(cell.key()), cell),
            cell(cell) {}

        /**
         * \brief Get the value of the cell.
         *
         * \return The value of the cell.
         */
        value_type value() const {
            return this->state->value();
        }

        /**
         * \brief Get the value of the cell and track it as a
         * dependency.
         *
         * \return The value of the cell.
         */
        value_type operator()() const {
            argument_tracker::global().track_argument(*this);
            return value();
        }

    private:

        /**
         * \brief The argument cell
         */
        const C cell;
    };

    /**
     * \brief Create a \p Cell that evaluates to the previous value of
     * \a cell.
     *
     * \attention The returned cell only starts tracking the previous
     * value of \a cell when the first observer is added. The cell
     * requires at least one observer to track the previous value of
     * \a cell correctly.
     *
     * \param cell cell The argument cell.
     *
     * \return A cell that evaluates to the previous value of \a cell.
     */
    template <Cell C>
    auto previous(const C &cell) {
        return previous_value_cell<C>(cell);
    }

    namespace ops {
        /**
         * \brief Operator for creating a cell that evaluates to the
         * previous value of another cell.
         *
         * \see live_cells::previous
         *
         * \param cell cell The argument cell.
         *
         * \return A cell that evaluates to the previous value of \a cell.
         */
        constexpr auto previous = [] (const Cell auto &cell) {
            return live_cells::previous(cell);
        };

    }  // ops

}  // live_cells

#endif /* LIVE_CELLS_PREVIOUS_VALUE_CELL_HPP */
