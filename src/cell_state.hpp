/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You
 * may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef LIVE_CELLS_CELL_STATE_HPP
#define LIVE_CELLS_CELL_STATE_HPP

#include <unordered_map>
#include <memory>

#include "keys.hpp"
#include "observable.hpp"

namespace live_cells {
    /**
     * \brief Maintains the state of a stateful cell.
     *
     * The state consists of the cell's observers and its value. The
     * base class provides functionality for keeping track of
     * observers added to the cell.
     *
     * This object is intended to be held in an std::shared_ptr with a
     * single instance per unique cell key. Multiple cell objects with
     * same key reference the same cell_state.
     *
     * A state is created when the first stateful cell with a given
     * key is created, and is destroyed when the last cell with the
     * given key is destroyed.
     */
    class cell_state : public std::enable_shared_from_this<cell_state> {
    public:
        /** \brief Shared pointer to a \p cell_state */
        typedef std::shared_ptr<cell_state> ref;

        /**
         * \brief Create a cell state with a given key.
         *
         * \param k The key
         */
        cell_state(key_ref k) : key_(k) {}

        virtual ~cell_state() noexcept;

        cell_state& operator=(const cell_state &other) = delete;

        /**
         * \brief Called before the first observer is added.
         *
         * Subclasses should override this method to include
         * initialization logic specific to this cell state.
         *
         * \note If the state needs to set up observers to other
         * cells, it should be done in this method.
         */
        virtual void init() {}

        /**
         * \brief Called after the last observer is removed.
         *
         * Subclasses should override this method to include cleanup
         * logic specific to this cell state.
         *
         * \note Observers added in \p init() should be removed in this
         * method.
         */
        virtual void pause() {}

        /**
         * \brief Add an observer to the cell's set of observers.
         *
         * \param o The observer to add
         */
        virtual void add_observer(observer::ref o);

        /**
         * \brief Remove an observer from the cell's set of observers.
         *
         * Like the \p remove_observer method of the \p Cell concept,
         * this method should be called the same number of times, for
         * a given observer \a o, as \p add_observer() was called,
         * before the observer is actually removed.
         *
         * \param o The observer to remove
         */
        virtual void remove_observer(observer::ref o);

        /**
         * \brief Notify the observers that the cell's value will change.
         *
         * This should be called before the value is changed.
         */
        virtual void notify_will_update();

        /**
         * \brief Notify the observers that the cell's value has changed.
         *
         * This should be called after the value has been changed.
         *
         * \param did_change True if the value of the cell may have
         *   changed, false if it is known that the value of the cell
         *   has not changed.
         */
        virtual void notify_update(bool did_change = true);

    protected:
        /**
         * \brief Key identifying the cell corresponding to this
         * state.
         */
        key_ref key_;

        /**
         * \brief The set of observers observing changes to the values
         * in the cell corresponding to this state.
         */
        std::unordered_map<observer::ref, std::size_t> observers;

        /**
         * \brief Does the cell have at least one observer?
         *
         * \return \p true if the cell has at least one observer.
         */
        bool is_active() const {
            return !observers.empty();
        }

    private:

#ifndef NDEBUG
        /**
         * \brief Number of times the observers were notified of
         * changes to this cell, during the current update cycle.
         *
         * This is incremented when \p notify_will_update() is called
         * and decremented when \p notify_update() is called. The
         * purpose of this counter is to ensure that the number of
         * times \p notify_will_update() is called matches the number
         * of times \p notify_update() is called. Ideally that should
         * only be once.
         *
         * \note This counter is only used for debugging when assert
         * is enabled, i.e. when \p NDEBUG is not defined.
         */
        int notify_count = 0;
#endif
    };

    /**
     * \brief Maintains the association between keys and cell states.
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
