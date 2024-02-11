#ifndef LIVE_CELLS_CELL_STATE_HPP
#define LIVE_CELLS_CELL_STATE_HPP

#include <unordered_map>
#include <memory>

#include "keys.hpp"
#include "cell.hpp"

namespace live_cells {
    /**
     * Maintains the state of a stateful cell.
     *
     * The state consists of the cell's observers and its value. The
     * base class provides functionality for keeping track of
     * observers added to the cell.
     *
     * This object is intended to be held in an std::shared_ptr with a
     * single instance per key. Multiple cell objects with same key
     * reference the same cell_state.
     *
     * A state is created when the first stateful cell with a given
     * key is created, and is destroyed when the last cell with the
     * given key is destroyed.
     */
    class cell_state : public std::enable_shared_from_this<cell_state> {
    public:
        typedef std::shared_ptr<cell_state> ref;

        /**
         * Create a cell state with a given key.
         *
         * @param k The key
         */
        cell_state(key_ref k) : key_(k) {}

        virtual ~cell_state() noexcept;

        cell_state& operator=(const cell_state &other) = delete;

        /**
         * Called before the first observer is added.
         *
         * If the state needs to set up observers to other cells, it
         * should be done in this method.
         */
        virtual void init() {}

        /**
         * Called after the last observer is removed.
         *
         * Observers added in init() should be removed in this method.
         */
        virtual void pause() {}

        /**
         * Add an observer to the cell's set of observers.
         *
         * @param o The observer
         */
        virtual void add_observer(observer::ref o);

        /**
         * Remove an observer from the cell's set of observers.
         *
         * Like observable::remove_observer(), this method should be
         * called the same number of times, for a given observer, as
         * add_observer() before the observer is actually removed.
         *
         * @param o The observer
         */
        virtual void remove_observer(observer::ref o);

        /**
         * Notify the observers that the cell's value will change.
         *
         * This should be called before the value is changed.
         */
        virtual void notify_will_update();

        /**
         * Notify the observers that the cell's value has changed.
         *
         * This should be called after the value has been changed.
         */
        virtual void notify_update();

    protected:
        /**
         * Key identifying this state.
         */
        key_ref key_;

        /**
         * Set of observers
         */
        std::unordered_map<observer::ref, std::size_t> observers;

        /**
         * Does the cell have at least one observer?
         */
        bool is_active() const {
            return !observers.empty();
        }
    };

    /**
     * Keeps track of the association between keys and cell states.
     */
    class state_manager {
    public:
        /**
         * Retrieve the global state_manager instance.
         */
        static state_manager &global() {
            return instance_;
        }

        /**
         * Retrieve a cell state associated with a given key, or
         * create a new state if there is no state associated with the
         * given key.
         *
         * A state of type @a S is created by passing one argument to
         * its constructor, the key.
         *
         * If @a k is a unique_key, or k->is_unique() is true, a new
         * state is always created without associating it with the
         * key.
         *
         * @param k The key
         *
         * @param args Additional arguments to pass to the constructor
         *   of the state.
         *
         * @return Reference to the state
         */
        template <typename S, typename... Args>
        std::shared_ptr<S> get(key_ref k, Args... args) {
            if (k->is_unique()) {
                return std::make_shared<S>(k, args...);
            }

            // TODO: Ensure that dynamic_pointer_cast succeeds.

            auto it = states.find(k);

            if (it != states.end()) {
                if (auto state = it->second.lock()) {
                    return std::dynamic_pointer_cast<S>(state);
                }
                else {
                    states.erase(it);
                }
            }

            auto state = states.emplace(k, std::weak_ptr<S>(std::make_shared<S>(k, args...))).first->second.lock();
            return std::dynamic_pointer_cast<S>(state);
        }

        /**
         * Remove the state associated with a given key.
         *
         * @param k The key
         */
        void remove(key_ref k) {
            if (!k->is_unique()) {
                states.erase(k);
            }
        }

    private:
        /** Singleton instance */
        static state_manager instance_;

        /**
         * State table
         */
        std::unordered_map<key_ref, std::weak_ptr<cell_state>> states;

        state_manager() = default;
        state_manager(const state_manager &) = delete;

        state_manager &operator=(const state_manager &) = delete;
    };

}  // live_cells

#endif /* LIVE_CELLS_CELL_STATE_HPP */
