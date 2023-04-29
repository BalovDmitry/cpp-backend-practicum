#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>
#include <optional>

#include "model_dog.h"
#include "model_map.h"

namespace model {

using DogPtr = std::shared_ptr<Dog>;

class GameSession {
public:
    GameSession(model::Map& map)
        : map_(map) {}

    GameSession(const GameSession& session) = default;
    GameSession& operator=(const GameSession& session) {
        name_to_dog_ = session.name_to_dog_;
        map_ = session.map_;
        return *this;
    }

    // Getters
    const auto& GetPlayers() const { return name_to_id_; }
    const auto& GetMapId() const { return map_.GetId(); }
    double GetMapSpeed() const { return map_.GetSpeed(); }
    std::optional<uint32_t> GetPlayerIdByName(const std::string& name);

    // Update state
    DogPtr AddDog(Position spawn_point, const std::string& name, uint32_t id);
    void UpdateTime(std::chrono::milliseconds delta);
    void UpdateDogPosition(DogPtr dog, std::chrono::milliseconds delta);

private:
    bool HasPlayerWithName(const std::string &name) { return name_to_id_.contains(name); }

private:
    std::unordered_map<std::string, DogPtr> name_to_dog_;
    std::unordered_map<std::string, uint32_t> name_to_id_;
    model::Map& map_;
};

}