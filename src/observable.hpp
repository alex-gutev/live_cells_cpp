#ifndef LIVE_CELLS_OBSERVABLE_H
#define LIVE_CELLS_OBSERVABLE_H

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
        key::ref key() const {
            return key_;
        }

    protected:
        /**
         * The key identifying this observable.
         */
        key::ref key_;
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
         * @param observable The observable.
         */
        template <typename T>
        observable_ref(T observable) :
            wrapped(wrap(observable)) {}

        observable_ref(const observable_ref &ref) :
            wrapped(ref.wrapped->clone()) {}

        observable_ref(observable_ref&& ref) :
            wrapped(std::move(ref.wrapped)) {
        }

        observable_ref &operator=(const observable_ref &ref) {
            wrapped = ref.wrapped->clone();
            return *this;
        }

        observable_ref &operator=(observable_ref&& ref) {
            wrapped = std::move(ref.wrapped);
            return *this;
        }

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

    private:
        /**
         * Extends the observable interface with a clone() method.
         */
        struct wrapper : observable {
            /**
             * Return a new wrapper holding an exact copy of the
             * observable held in this wrapper.
             */
            virtual std::unique_ptr<wrapper> clone() const = 0;
        };

        /**
         * A wrapper holding an observable of type @a T.
         */
        template <typename T>
        struct t_wrapper : wrapper {
            /**
             * Create a wrapper holding @a observable.
             *
             * @param observable The observable.
             */
            t_wrapper(T observable) :
                observable_(observable) {}

            std::unique_ptr<wrapper> clone() const override {
                auto ptr = std::make_unique<t_wrapper>(observable_);

                return std::unique_ptr<wrapper>(
                    static_cast<wrapper*>(ptr.release())
                );
            }

            void add_observer(observer::ref o) override {
                observable_.add_observer(o);
            }

            void remove_observer(observer::ref o) override {
                observable_.remove_observer(o);
            }

        private:
            /** Observable held by this wrapper */
            T observable_;

        };

        /**
         * Pointer to the wrapper holding the observable.
         */
        std::unique_ptr<wrapper> wrapped;

        /**
         * Create a polymorphic wrapper holding an observable.
         *
         * @param observable The observable
         *
         * @return Pointer to a wrapper holding a copy of @a
         *   observable.
         */
        template <typename T>
        static std::unique_ptr<wrapper> wrap(T observable) {
            auto ptr = std::make_unique<t_wrapper<T>>(observable);

            return std::unique_ptr<wrapper>(
                static_cast<wrapper*>(ptr.release())
            );
        }
    };

    /**
     * Compare two observables for equality by their keys.
     *
     * The comparison is done using key::eq.
     *
     * @param a An observable
     * @param b An observable
     *
     * @return true if @a and @b represent the same observable.
     */
    inline bool operator ==(const observable &a, const observable &b) {
        return a.key()->eq(*b.key());
    }

    /**
     * Check that two observables are not equal.
     *
     * The comparison is done using key::eq on the keys of the
     * observables.
     *
     * @param a An observable
     * @param b An observable
     *
     * @return true if @a and @b do represent the same observable.
     */
    inline bool operator !=(const observable &a, const observable &b) {
        return !(a == b);
    }

    /**
     * Compares to polymorphic observables by their keys.
     *
     * @param a A polymorphic observable
     * @param b A polymorphic observable
     *
     * @return true if the key of @a a is equal to the key of @a b.
     */
    inline bool operator==(const observable_ref &a, const observable_ref &b) {
        key_equality cmp;
        return cmp(a->key(), b->key());
    }

}  // live_cells

template<>
struct std::hash<live_cells::observable> {
    std::size_t operator()(const live_cells::observable &a) const noexcept {
        return a.key()->hash();
    }
};

template<>
struct std::hash<live_cells::observable_ref> {
    std::size_t operator()(const live_cells::observable_ref &a) const noexcept {
        live_cells::key_hash h;
        return h(a->key());
    }
};

#endif /* LIVE_CELLS_OBSERVABLE_H */
