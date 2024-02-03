#ifndef LIVE_CELLS_CELL_HPP
#define LIVE_CELLS_CELL_HPP

#include <memory>

namespace live_cells {

    class observable;

    /**
     * Defines the observer interface for observing a cell.
     */
    class observer {
    public:
        /**
         * observer pointer type
         */
        typedef std::shared_ptr<observer> ref;

        virtual ~observer() noexcept = default;

        /**
         * Notifies this observer that the value of the observable @a
         * o is going to change.
         *
         * @param o The cell observable
         */
        virtual void will_update(const observable &o) = 0;

        /**
         * Notifies this observer that the value of observable @a o
         * has changed.
         *
         * @param o The cell observable.
         */
        virtual void update(const observable &o) = 0;
    };

    /**
     * Base observable interface.
     *
     * This interface provides methods for adding and removing
     * observers.
     */
    class observable {
    public:
        virtual ~observable() noexcept = default;

        /**
         * Add an observer for this observable.
         *
         * The observer is notified of changes to this observable's
         * value.
         *
         * @param o The observer.
         */
        virtual void add_observer(observer::ref o) = 0;

        /**
         * Remove an observer for this observable.
         *
         * The observer, which was previously added using @a
         * add_observer(), is removed and no longer notified of
         * changes to this observable's value.
         *
         * **NOTE**: If @a add_observer() was called more than once
         * for this observer, it @a remove_observer() must be called
         * the same number of times before the observer is actually
         * removed.
         *
         * @param o The observer.
         */
        virtual void remove_observer(observer::ref o) = 0;
    };

    /**
     * Base cell interface.
     *
     * A cell is an observable with a value. This interface provides
     * an accessor for retrieving the cell's value.
     */
    template <typename T>
    class cell : observable {
    public:

        /**
         * Get the value of the cell.
         *
         * @return The value
         */
        virtual T value() const = 0;
    };

}  // live_cells

#endif /* LIVE_CELLS_CELL_HPP */
