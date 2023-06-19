#pragma once

#include "model_utils.h"

namespace model {

static const double LOOT_WIDTH = 0.0;

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