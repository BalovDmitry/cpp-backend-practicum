#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>

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
    const auto& GetMapId() const { return map_.GetId(); }
    double GetMapSpeed() const { return map_.GetSpeed(); }

    // Update state
    DogPtr AddDog(Position spawn_point, const std::string& name);
    void UpdateTime(std::chrono::milliseconds delta);
    void UpdateDogPosition(DogPtr dog, std::chrono::milliseconds delta);

private:
    std::unordered_map<std::string, DogPtr> name_to_dog_;
    model::Map& map_;
};

}