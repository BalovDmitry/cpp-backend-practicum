#pragma once

#include <boost/json/parse.hpp>

#include "model_map.h"
#include "game_server_exceptions.h"

namespace model {

class ExtraData {
public:
    static ExtraData& GetInstance() {
        static ExtraData instance;
        return instance;
    }

    ExtraData(const ExtraData& other) = delete;
    ExtraData(ExtraData&& other) = delete;

public:
    // Setters
    bool AddLootToMap(const model::Map::Id& id, const boost::json::array& loot);
    void SetLootGeneratorData(double period, double probability);

    // Getters
    decltype(auto) GetLootByMapId(const model::Map::Id& id) const {
        if (map_to_loot_.contains(id)) {
            return map_to_loot_.at(id);
        }
        throw server_exceptions::InvalidMapException();
    }

    const auto& GetLootGeneratorData() const {
        return loot_generator_data_;
    }

private:
    ExtraData() = default;

private:
    struct LootGeneratorData {
        long long period = 0.0;
        double probability = 0.0;
    };

    using MapIdToLoot = std::unordered_map<model::Map::Id, boost::json::array, util::TaggedHasher<model::Map::Id>>;
    MapIdToLoot map_to_loot_;
    LootGeneratorData loot_generator_data_;
};

}
