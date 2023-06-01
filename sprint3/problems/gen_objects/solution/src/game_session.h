#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>
#include <optional>

#include "model_dog.h"
#include "model_map.h"
#include "extra_data.h"
#include "loot_generator.h"

namespace model {

using DogPtr = std::shared_ptr<Dog>;

class GameSession {
public:
    GameSession(model::Map& map, unsigned loot_size = 0)
        : map_(map) {
        
        loot_size_ = model::ExtraData::GetInstance().GetLootByMapId(map.GetId()).size();
        using namespace std::chrono_literals;
        loot_generator_ = std::make_shared<loot_gen::LootGenerator>(
            model::ExtraData::GetInstance().GetLootGeneratorData().period * 1ms, 
            model::ExtraData::GetInstance().GetLootGeneratorData().probability);
    }

    GameSession(const GameSession& session) = default;
    GameSession& operator=(const GameSession& session) {
        name_to_dog_ = session.name_to_dog_;
        name_to_id_ = session.name_to_id_;
        map_ = session.map_;
        loot_generator_ = session.loot_generator_;
        loot_count_ = session.loot_count_;
        return *this;
    }

    // Getters
    const auto& GetPlayers() const { return name_to_id_; }
    const auto& GetMapId() const { return map_.GetId(); }
    double GetMapSpeed() const { return map_.GetSpeed(); }
    unsigned GetLootCount() const { return loot_count_; }
    const auto& GetLootObject() const { return loot_object_; }
    std::optional<uint32_t> GetPlayerIdByName(const std::string& name);

    // Update state
    DogPtr AddDog(Position spawn_point, const std::string& name, uint32_t id);
    void UpdateTime(std::chrono::milliseconds delta);
    void UpdateDogPosition(DogPtr dog, std::chrono::milliseconds delta);
    void UpdateLostObjects(std::chrono::milliseconds delta);

private:
    bool HasPlayerWithName(const std::string &name) { return name_to_id_.contains(name); }

private:
    std::unordered_map<std::string, DogPtr> name_to_dog_;
    std::unordered_map<std::string, uint32_t> name_to_id_;
    model::Map& map_;

    // Fields for loot
    std::shared_ptr<loot_gen::LootGenerator> loot_generator_;
    unsigned loot_count_ = 0;
    unsigned loot_size_ = 0;
    boost::json::object loot_object_;
};

}