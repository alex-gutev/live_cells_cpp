#ifndef LIVE_CELLS_CELL_HPP
#define LIVE_CELLS_CELL_HPP

#include <memory>

#include "keys.hpp"

namespace live_cells {

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
         * Notifies this observer that the value of the observable
         * identified by @a k is going to change.
         *
         * @param Key identifying observable
         */
        virtual void will_update(const key::ref &k) = 0;

        /**
         * Notifies this observer that the value of the observable
         * identified by @a k has changed.
         *
         * @param Key identifying observable
         */
        virtual void update(const key::ref &k) = 0;
    };

    /**
     * Base observable interface.
     *
     * This interface provides methods for adding and removing
     * observers.
     */
    class observable {
    public:
        /**
         * Construct an observable with a unique_key.
         */
        observable() : observable(std::make_shared<unique_key>()) {}

        /**
         * Construct an observable with a given key @a k.
         *
         * @param k The key
         */
        template <typename K>
        observable(std::shared_ptr<K> k) :
            key_(std::static_pointer_cast<const live_cells::key>(k)) {}

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

        /**
         * Retrieve the key that uniquely identifies this cell.
         */
        key::ref key() {
            return key_;
        }

    protected:
        /**
         * The key identifying this observable.
         */
        key::ref key_;
    };

    /**
     * Base cell interface.
     *
     * A cell is an observable with a value. This interface provides
     * an accessor for retrieving the cell's value.
     */
    template <typename T>
    class cell : public observable {
    public:
        using observable::observable;

        /**
         * Get the value of the cell.
         *
         * @return The value
         */
        virtual T value() const = 0;
    };

}  // live_cells

#endif /* LIVE_CELLS_CELL_HPP */
