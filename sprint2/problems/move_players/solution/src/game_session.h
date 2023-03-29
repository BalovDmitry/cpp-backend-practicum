#pragma once

#include <unordered_map>
#include <memory>

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
    const auto& GetMapId() const { return map_.GetId(); }
    double GetMapSpeed() const { return map_.GetSpeed(); }
    DogPtr AddDog(Position spawn_point, const std::string& name) {
        auto dog = std::make_shared<Dog>(name);
        dog->SetPosition(spawn_point);
        dog->SetSpeed(map_.GetSpeed());
        name_to_dog_[name] = dog;
        return name_to_dog_.at(name);
    }

private:
    std::unordered_map<std::string, DogPtr> name_to_dog_;
    model::Map& map_;
};

}