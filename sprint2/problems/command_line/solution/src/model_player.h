#pragma once

#include <string>

#include "model_map.h"
#include "model_dog.h"
#include "model_utils.h"
#include "game_session.h"

namespace model {

using SessionPtr = std::shared_ptr<GameSession>;
using DogPtr = std::shared_ptr<Dog>;

class Player {
public:
    Player(const std::string& name, uint32_t id, model::Map::Id map_id, DogPtr dog)
        : name_(name)
        , map_id_(map_id) 
        , dog_(dog) 
        , id_(id) {}

    // Getters
    uint32_t GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    const auto& GetMapId() const { return map_id_; }
    DogPtr GetDog() { return dog_; }
    const DogPtr GetDog() const { return dog_; }

private:
    uint32_t id_;
    std::string name_;
    model::Map::Id map_id_;
    DogPtr dog_;
};

}