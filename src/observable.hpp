#ifndef LIVE_CELLS_OBSERVABLE_H
#define LIVE_CELLS_OBSERVABLE_H

#include "keys.hpp"

namespace live_cells {

    template <typename T>
    class cell;

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
        virtual void will_update(const key_ref &k) = 0;

        /**
         * Notifies this observer that the value of the observable
         * identified by @a k has changed.
         *
         * @param Key identifying observable
         */
        virtual void update(const key_ref &k) = 0;
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
        observable() : observable(key_ref::create<unique_key>()) {}

        /**
         * Construct an observable with a given key @a k.
         *
         * @param k The key
         */
        observable(key_ref k) :
            key_(k) {}

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
         * **NOTE**:
         *
         * If @a add_observer() was called more than once
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
        key_ref key() const {
            return key_;
        }

    protected:
        /**
         * The key identifying this observable.
         */
        key_ref key_;
    };

    /**
     * Polymorphic observable holder which allows an observable to be
     * copied while preserving its runtime type.
     *
     * A statically typed observable cannot be copied or stored in a
     * container, such as std::vector, unless its exact type is known
     * at compile-time. This class wraps an observable and allows it
     * to be copied while preserving its type.
     *
     * **NOTE**:
     *
     * When an observable_ref is copied, the underlying
     * observable is still copied but its type is preserved.
     */
    class observable_ref {
    public:
        /**
         * Create a polymorphic holder for @a observable.
         *
         * @param observable Pointer to the observable.
         */
        observable_ref(std::shared_ptr<observable> observable) :
            wrapped(observable) {}

        /**
         * Create a polymorphic holder for @a observable.
         *
         * @param observable The observable.
         */
        template <typename T>
        observable_ref(T o) :
            wrapped(std::static_pointer_cast<observable>(std::make_shared<T>(o))) {}

        /**
         * Provide access to the wrapped observable.
         */
        observable *operator ->() {
            return wrapped.get();
        }

        /**
         * Provide access to the wrapped observable.
         */
        const observable *operator ->() const {
            return wrapped.get();
        }

        /**
         * Retrieve the value held by the observable.
         *
         * This method attempts to cast the reference to the
         * underlying observable, to cell<T>.
         *
         * @return The value held by the observable.
         *
         * @throws std::bad_cast if the observable does not hold a
         *   value of type T.
         */
        template <typename T>
        T value() {
            const observable &obs_ref = *wrapped;
            const cell<T> &value_ref = dynamic_cast<const cell<T> &>(obs_ref);

            return value_ref.value();
        }

    private:
        /**
         * Pointer to the wrapper holding the observable.
         */
        std::shared_ptr<observable> wrapped;

    };

    /**
     * Compares to polymorphic observables by their keys.
     *
     * @param a A polymorphic observable
     * @param b A polymorphic observable
     *
     * @return true if the key of @a a is equal to the key of @a b.
     */
    inline bool operator==(const observable_ref &a, const observable_ref &b) {
        return a->key() == b->key();
    }

    inline bool operator!=(const observable_ref &a, const observable_ref &b) {
        return !(a == b);
    }

}  // live_cells

template<>
struct std::hash<live_cells::observable_ref> {
    std::size_t operator()(const live_cells::observable_ref &a) const noexcept {
        return a->key()->hash();
    }
};

#endif /* LIVE_CELLS_OBSERVABLE_H */
