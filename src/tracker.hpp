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

#ifndef LIVE_CELLS_TRACKER_HPP
#define LIVE_CELLS_TRACKER_HPP

#include <functional>

#include "observable.hpp"

namespace live_cells {

    /**
     * Helper for dynamically tracking the argument cells used within
     * a function.
     */
    class argument_tracker {
    public:
        /**
         * Track argument callback function.
         *
         * @param arg The referenced observable.
         */
        typedef std::function<void(const cell &arg)> track_fn;

        /**
         * Registers a track argument callback function on
         * construction, and restores the previous callback function
         * on destruction.
         */
        template <typename T>
        class tracker {
        public:
            /**
             * Register @a fn as the track argument callback function
             * for argument tracker @a tracker.
             *
             * The previous callback is restored when this object is
             * destroyed.
             *
             * @param fn      Track argument callback function
             * @param tracker Argument cell tracker
             */
            tracker(T fn, argument_tracker &tracker) :
                tracker_(tracker),
                previous(tracker.track_callback) {
                tracker_.track_callback = track_fn(fn);
            }

            tracker(const tracker &) = delete;
            tracker(tracker &&other) :
                tracker_(other.tracker_),
                previous(other.previous) {
                other.moved = true;
            }

            tracker &operator=(const tracker &) = delete;
            tracker &operator=(tracker &&other) {
                tracker_ = std::move(other.tracker_);
                previous = std::move(other.previous);
            }

            /**
             * Restores the previous track argument callback.
             */
            ~tracker() {
                if (!moved)
                    tracker_.track_callback = previous;
            }

        private:
            /** Argument tracker */
            argument_tracker &tracker_;

            /** Previous callback function */
            track_fn previous;

            /** Has this object been moved */
            bool moved = false;
        };

        /**
         * Return the global argument tracker instance.
         */
        static argument_tracker& global() {
            return instance_;
        }

        argument_tracker(const argument_tracker &other) = delete;
        argument_tracker& operator=(const argument_tracker &other) = delete;

        /**
         * Inform the current argument tracker that cell @a arg was
         * used.
         *
         * @param arg A cell that was referenced as an argument.
         */
        void track_argument(const cell &arg);

        /**
         * Register a track argument callback function.
         *
         * @param track Track argument callback function.
         *
         * @return A tracker object, which automatically restores the
         *   previous callback, at the time of calling this method,
         *   when it is destroyed.
         */
        template <typename F>
        tracker<F> with_tracker(F track) {
            return tracker<F>(track, *this);
        }

    private:
        /** Singleton instance */
        static argument_tracker instance_;

        /** Current track argument callback */
        track_fn track_callback;

        argument_tracker() = default;
    };

}  // live_cells

#endif /* LIVE_CELLS_TRACKER_HPP */
