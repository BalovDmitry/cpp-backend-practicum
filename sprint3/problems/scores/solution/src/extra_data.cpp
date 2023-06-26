#include "extra_data.h"
#include "logger.h"

namespace model {

bool ExtraData::AddLootToMap(const model::Map::Id& id, const boost::json::array& loot) {
    if (map_to_loot_.contains(id)) {
        logger::LogErrorMessage("Loot has been already added to map.");
        return false;
    }
    map_to_loot_[id] = loot;
    
    try {
        auto& loot_values = map_to_loot_values_[id];
        for (int i = 0; i < loot.size(); ++i) {
            auto loot_obj = loot[i].as_object();
            loot_values.push_back(loot_obj.at("value").as_int64());
        }
    } catch (const std::exception& e) {
        logger::LogErrorMessage(e.what());
        return false;
    }

    return true;
}

void ExtraData::SetLootGeneratorData(double period, double probability) {
    loot_generator_data_.period = period;
    loot_generator_data_.probability = probability;
}

}

