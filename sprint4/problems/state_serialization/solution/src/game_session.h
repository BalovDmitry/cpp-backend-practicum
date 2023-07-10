#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>
#include <optional>

#include "model_dog.h"
#include "model_map.h"
#include "model_loot_item.h"
#include "extra_data.h"
#include "loot_generator.h"
#include "collision_detector.h"

namespace model {

using DogPtr = std::shared_ptr<Dog>;

class GameSession {
public:
    GameSession(model::Map& map, std::optional<unsigned> loot_size = std::nullopt)
        : map_(map) {
        
        // Option for testing
        if (loot_size.has_value()) {
            loot_size_ = loot_size.value();
        } else {
            loot_size_ = model::ExtraData::GetInstance().GetLootValuesByMapId(map.GetId()).size();
        }

        SetLootGeneratorData(model::ExtraData::GetInstance().GetLootGeneratorData().period, model::ExtraData::GetInstance().GetLootGeneratorData().probability);
    }

    GameSession(const GameSession& session) = default;
    GameSession& operator=(const GameSession& session) {
        name_to_id_ = session.name_to_id_;
        id_to_dog_ = session.id_to_dog_;
        available_loot_items_ = session.available_loot_items_;
        map_ = session.map_;
        loot_generator_ = session.loot_generator_;
        return *this;
    }

    // Getters
    const auto& GetPlayers() const { return name_to_id_; }
    const auto& GetMapId() const { return map_.GetId(); }
    double GetMapSpeed() const { return map_.GetSpeed(); }
    const auto& GetAvailableLoot() const { return available_loot_items_; }
    int GetPlayerScore(uint32_t id) const { return id_to_dog_.at(id)->GetScore(); }
    std::optional<uint32_t> GetPlayerIdByName(const std::string& name);
    std::chrono::milliseconds GetTimeSinceSave() const { return time_since_save_; }

    // Setters
    void SetLootGeneratorData(double base_interval, double probability);
    void SetTimeSinceSave(std::chrono::milliseconds time_since_save);

    // Update state
    DogPtr AddDog(Position spawn_point, const std::string& name, uint32_t id);
    void UpdateTime(std::chrono::milliseconds delta);
    void UpdateDogPosition(DogPtr dog, std::chrono::milliseconds delta);
    void UpdateLostObjects(std::chrono::milliseconds delta);

private:
    bool HasPlayerWithName(const std::string &name) { return name_to_id_.contains(name); }
    void TryGenerateLoot(std::chrono::milliseconds delta);
    void UpdateLootProvider();
    void UpdateCollisions();
    bool TryUpdateCollisionsWithOffice(const collision_detector::GatheringEvent& gather_event);
    bool TryUpdateCollisionsWithLoot(const collision_detector::GatheringEvent& gather_event);

private:
    std::unordered_map<std::string, uint32_t> name_to_id_;
    std::unordered_map<uint32_t, DogPtr> id_to_dog_;
    std::unordered_map<uint32_t, LootItem> available_loot_items_;
    model::Map& map_;

    // Fields for loot
    std::shared_ptr<loot_gen::LootGenerator> loot_generator_;
    unsigned loot_size_ = 0;
    unsigned loot_id_ = 0;

    collision_detector::ItemGathererProviderBase loot_provider_;

    // Field to save state
    std::chrono::milliseconds time_since_save_{0};
};

}