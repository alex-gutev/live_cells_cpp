/*
 * live_cells_cpp
 * Copyright (C) 2024  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
        typedef std::function<void(const observable_ref &arg)> track_fn;

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
        void track_argument(const observable_ref &arg);

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
