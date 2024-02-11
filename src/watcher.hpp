#ifndef LIVE_CELLS_WATCHER_HPP
#define LIVE_CELLS_WATCHER_HPP

#include <utility>
#include <memory>
#include <unordered_set>

#include "keys.hpp"
#include "cell.hpp"
#include "tracker.hpp"

namespace live_cells {

    /**
     * Handle for a cell watch function.
     *
     * This class registers a cell watch function on construction and
     * automatically removes it on destruction. The watch function can
     * also be removed prior to destruction with the stop() method.
     */
    template <typename F>
    class watcher : public observer, public std::enable_shared_from_this<watcher<F>> {
    public:

        /**
         * Register a cell watch function @a callback.
         *
         * @param callback A cell watch function.
         */
        watcher(F callback) : callback(callback) {}
        watcher(const watcher &) = delete;

        /**
         * Remove the registered cell watch function
         */
        ~watcher() {
            stop();
        }

        watcher &operator=(const watcher &) = delete;

        /**
         * Remove the watch function.
         *
         * The watch function will no longer be called after calling
         * this method.
         */
        void stop() {
            for (auto arg : arguments) {
                arg->remove_observer(
                    std::static_pointer_cast<observer>(this->shared_from_this())
                );
            }

            arguments.clear();
        }

        template <typename U>
        friend std::shared_ptr<watcher<U>> watch(U fn);

    private:

        /** Cell watch function */
        const F callback;

        /** Argument cells referenced by watch function */
        std::unordered_set<observable_ref> arguments;

        /**
         * Is an argument cell value update currently in progresss?
         */
        bool is_updating = false;

        /* observer methods */

        void will_update(const key_ref &k) override {
            is_updating = true;
        }

        void update(const key_ref &k) override {
            if (is_updating) {
                is_updating = false;
                call_with_tracker();
            }
        }

        /**
         * Call the watch function and dynamically determine its
         * arguments.
         */
        void call_with_tracker() {
            auto t = argument_tracker::global().with_tracker([this] (auto cell) {
                if (!arguments.count(cell)) {
                    arguments.emplace(cell);
                    cell->add_observer(
                        std::static_pointer_cast<observer>(this->shared_from_this())
                    );
                }
            });

            callback();
        }
    };

    /**
     * Register a cell watch function.
     *
     * The function @a fn is called whenever the values of the cells,
     * referenced by it, change.
     *
     * @a fn is always called once immediately before this function
     * returns.
     *
     * @param fn Watch function
     *
     * @return A shared pointer to a watch handle. The watch function
     *   is called for all changes to the values of the argument cells
     *   until the watch handle is destroyed, or its stop() method is
     *   called.
     */
    template <typename F>
    std::shared_ptr<watcher<F>> watch(F fn) {
        auto w = std::make_shared<watcher<F>>(fn);
        w->call_with_tracker();

        return w;
    }

}  // live_cells

#endif /* LIVE_CELLS_WATCHER_HPP */
