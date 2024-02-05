#include "cell_state.hpp"

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
