#include "extra_data.h"
#include "logger.h"

namespace model {

bool ExtraData::AddLootToMap(const model::Map::Id& id, const boost::json::array& loot) {
    if (map_to_loot_.contains(id)) {
        logger::LogErrorMessage("Loot has been already added to map.");
        return false;
    }
    map_to_loot_[id] = loot;
    return true;
}

void ExtraData::SetLootGeneratorData(double period, double probability) {
    loot_generator_data_.period = period;
    loot_generator_data_.probability = probability;
}

}

