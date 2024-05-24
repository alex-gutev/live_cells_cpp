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

#ifndef LIVE_CELLS_WATCHER_HPP
#define LIVE_CELLS_WATCHER_HPP

#include <utility>
#include <memory>
#include <unordered_set>

#include "keys.hpp"
#include "observable.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * \brief Handle for a cell watch function.
     *
     * This class registers a cell watch function on construction and
     * automatically removes it on destruction. The watch function can
     * also be removed prior to destruction with the \p stop() method.
     */
    class watcher : public std::enable_shared_from_this<watcher> {
    public:

        /**
         * \brief Register the cell watch function \a callback.
         *
         * \a callback is called whenever the values of the \p Cell's
         * referenced with it, using the function call operator,
         * change. It is called once immediately, before the
         * constructor returns, to determine which cells are
         * referenced.
         *
         * \param callback A function of no arguments.
         */
        template <typename F>
        watcher(F callback) :
            observer(
                std::static_pointer_cast<base_observer>(
                    std::make_shared<watch_observer<F>>(callback)
                )
            ) {
            observer->init();
        }

        watcher(const watcher &) = delete;

        /**
         * \brief Remove the registered cell watch function.
         */
        ~watcher() {
            stop();
        }

        watcher &operator=(const watcher &) = delete;

        /**
         * \brief Remove the watch function.
         *
         * The watch function will no longer be called after calling
         * this method.
         */
        void stop() {
            observer->stop();
        }

    private:

        /**
         * \brief Base watch function observer type.
         */
        struct base_observer : public observer, public std::enable_shared_from_this<base_observer> {

            /** \brief Argument cells referenced by watch function */
            std::unordered_set<cell> arguments;

            virtual ~base_observer() = default;

            /**
             * \brief Call watch function to determine dependencies.
             */
            virtual void init() = 0;

            /**
             * \brief Remove the watch function.
             *
             * The watch function will no longer be called after calling
             * this method.
             */
            void stop() {
                for (auto arg : arguments) {
                    arg.remove_observer(
                        std::static_pointer_cast<observer>(this->shared_from_this())
                    );
                }

                arguments.clear();
            }
        };

        /**
         * \brief The observer which calls the watch function when the
         * cell values change.
         */
        template <typename F>
        struct watch_observer : public base_observer {
            /** \brief Cell watch function */
            const F callback;

            watch_observer(F callback) : callback(callback) {}

            watch_observer(const watch_observer &) = delete;

            void init() override {
                call_with_tracker();
            }

            /**
             * \brief Is an argument cell value update currently in
             * progresss?
             */
            bool is_updating = false;

            /**
             * \brief Is this observer waiting for a call to \p
             * update() with \c changed equal to \c false?
             */
            bool waiting_for_change = false;


            /* observer methods */

            void will_update(const key_ref &k) override {
                if (!is_updating) {
                    is_updating = true;
                    waiting_for_change = false;
                }
            }

            void update(const key_ref &k, bool changed) override {
                if (is_updating || (changed && waiting_for_change)) {
                    is_updating = false;
                    waiting_for_change = !changed;

                    if (changed) {
                        call_with_tracker();
                    }
                }
            }

            /**
             * \brief Call the watch function and dynamically
             * determine its arguments.
             */
            void call_with_tracker() {
                auto t = argument_tracker::global().with_tracker([this] (auto cell) {
                    if (!this->arguments.count(cell)) {
                        this->arguments.emplace(cell);
                        cell.add_observer(
                            std::static_pointer_cast<observer>(this->shared_from_this())
                        );
                    }
                });

                try {
                    callback();
                }
                catch (...) {
                    // TODO: Log exception to stderr in debug mode
                    // Prevent exception from being propagated to caller
                }
            }
        };

        /** \brief The cell observer that calls the watch function */
        std::shared_ptr<base_observer> observer;
    };

    /**
     * \brief Register a cell watch function.
     *
     * The function \a fn is called whenever the values of the cells
     * referenced within it change.
     *
     * \a fn is always called once immediately before this function
     * returns, to determine which cells it references.
     *
     * \param fn A function of no arguments.
     *
     * \return A shared pointer to a watch handle.\n The watch function
     *   is called for all changes to the values of the argument cells
     *   until the watch handle is destroyed, or its \p
     *   watcher::stop() method is called.
     */
    template <typename F>
    std::shared_ptr<watcher> watch(F fn) {
        auto w = std::make_shared<watcher>(fn);

        return w;
    }

}  // live_cells

#endif /* LIVE_CELLS_WATCHER_HPP */
