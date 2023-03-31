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
    Player(const std::string& name, u_int32_t id, SessionPtr session, DogPtr dog)
        : name_(name)
        , session_(session) 
        , dog_(dog) 
        , id_(id) {
        }

    // Getters
    const uint32_t GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    const auto& GetMapId() const { return session_->GetMapId(); }
    DogPtr GetDog() { return dog_; }
    const DogPtr GetDog() const { return dog_; }

private:
    u_int32_t id_;
    std::string name_;
    SessionPtr session_;
    DogPtr dog_;
};

}