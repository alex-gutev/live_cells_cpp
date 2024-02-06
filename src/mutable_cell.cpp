#include "mutable_cell.hpp"

#include <vector>
#include <iostream>

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
