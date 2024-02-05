#include "tracker.hpp"
#include "cell.hpp"

live_cells::argument_tracker live_cells::argument_tracker::instance_ = {};

void live_cells::argument_tracker::track_argument(const observable_ref &arg) {
    if (track_callback) {
        track_callback(arg);
    }
}
