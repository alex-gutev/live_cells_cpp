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
            return wrapped->get();
        }

        /**
         * Provide access to the wrapped observable.
         */
        const observable *operator ->() const {
            return wrapped->get();
        }

    private:
        /**
         * Holds an observable and provides a clone() method.
         */
        struct holder {
            /**
             * Return a new wrapper holding an exact copy of the
             * observable held in this wrapper.
             */
            virtual std::unique_ptr<holder> clone() const = 0;

            /**
             * Return a pointer to the underlying observable.
             */
            virtual observable *get() = 0;
            virtual const observable *get() const = 0;
        };

        /**
         * A wrapper holding an observable of type @a T.
         */
        template <typename T>
        struct t_holder : holder {
            /**
             * Create a wrapper holding @a observable.
             *
             * @param observable The observable.
             */
            t_holder(T observable) :
                observable_(observable) {}

            std::unique_ptr<holder> clone() const override {
                auto ptr = std::make_unique<t_holder>(observable_);

                return std::unique_ptr<holder>(
                    static_cast<holder*>(ptr.release())
                );
            }

            observable *get() override {
                return &observable_;
            }

            const observable *get() const override {
                return &observable_;
            }

        private:
            /** Observable held by this wrapper */
            T observable_;

        };

        /**
         * Pointer to the wrapper holding the observable.
         */
        std::unique_ptr<holder> wrapped;

        /**
         * Create a polymorphic wrapper holding an observable.
         *
         * @param observable The observable
         *
         * @return Pointer to a wrapper holding a copy of @a
         *   observable.
         */
        template <typename T>
        static std::unique_ptr<holder> wrap(T observable) {
            auto ptr = std::make_unique<t_holder<T>>(observable);

            return std::unique_ptr<holder>(
                static_cast<holder*>(ptr.release())
            );
        }
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
        key_equality cmp;
        return cmp(a->key(), b->key());
    }

    inline bool operator!=(const observable_ref &a, const observable_ref &b) {
        return !(a == b);
    }

}  // live_cells

template<>
struct std::hash<live_cells::observable_ref> {
    std::size_t operator()(const live_cells::observable_ref &a) const noexcept {
        live_cells::key_hash h;
        return h(a->key());
    }
};

#endif /* LIVE_CELLS_OBSERVABLE_H */
