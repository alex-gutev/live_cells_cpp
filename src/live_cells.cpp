#include <vector>
#include <iostream>

#include "keys.hpp"
#include "cell_state.hpp"
#include "mutable_cell.hpp"
#include "tracker.hpp"
#include "cell.hpp"

/* Keys */

bool live_cells::unique_key::eq(const key &k) const noexcept {
    return this == &k;
}

std::size_t live_cells::unique_key::hash() const noexcept {
    std::hash<const unique_key*> hash;

    return hash(this);
}

/* Cell States */

live_cells::state_manager live_cells::state_manager::instance_ = {};

live_cells::cell_state::~cell_state() noexcept  {
    state_manager::global().remove(key_);
}

void live_cells::cell_state::add_observer(observer::ref o) {
    if (observers.empty()) {
        init();
    }

    auto it = observers.find(o);

    if (it != observers.end()) {
        it->second++;
    }
    else {
        observers.emplace(o, 1);
    }
}

void live_cells::cell_state::remove_observer(observer::ref o) {
    auto it = observers.find(o);

    if (it != observers.end()) {
        if ((it->second -= 1) == 0) {
            observers.erase(it);

            if (observers.empty()) {
                pause();
            }
        }
    }
}

void live_cells::cell_state::notify_will_update() {
    for (auto entry : observers) {
        try {
            entry.first->will_update(key_);
        }
        catch (...) {
            // Prevent exception from being propagated to caller
        }
    }
}

void live_cells::cell_state::notify_update() {
    for (auto entry : observers) {
        try {
            entry.first->update(key_);
        }
        catch (...) {
            // Prevent exception from being propagated to caller
        }
    }
}

/* Batching */

/**
 * Is a batch update currently in effect?
 */
static bool is_batch_update = false;

/**
 * List of cell states which have had their values modified during the
 * current batch.
 */
static std::vector<std::shared_ptr<live_cells::cell_state>> batch_list;

bool live_cells::batch_update::is_batch_update() {
    return ::is_batch_update;
}

void live_cells::batch_update::add_to_batch(std::shared_ptr<cell_state> state) {
    batch_list.push_back(state);
}

live_cells::batch_update::batch_update() : is_batching(!is_batch_update()) {
    if (is_batching) {
        ::is_batch_update = true;
    }
}

live_cells::batch_update::~batch_update() {
    if (is_batching) {
        std::cout << "Here" << "\n";
        ::is_batch_update = false;

        for (auto state : batch_list) {
            state->notify_update();
        }

        batch_list.clear();
    }
}

/* Dynamic dependency teacking */

live_cells::argument_tracker live_cells::argument_tracker::instance_ = {};

void live_cells::argument_tracker::track_argument(const observable_ref &arg) {
    if (track_callback) {
        track_callback(arg);
    }
}
