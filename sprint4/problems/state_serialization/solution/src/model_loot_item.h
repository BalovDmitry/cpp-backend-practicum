#pragma once

#include "model_utils.h"

namespace model {

struct LootItem {
    LootItem() = default;
    LootItem(unsigned type, const Position& position) {
        this->type = type;
        this->position = position;
    }

    unsigned type;
    Position position;
};

};