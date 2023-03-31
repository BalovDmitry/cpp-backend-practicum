#include "game_session.h"

namespace model {

DogPtr GameSession::AddDog(Position spawn_point, const std::string& name) {
    auto dog = std::make_shared<Dog>(name, spawn_point, map_.GetSpeed(), Direction::NORTH);
    name_to_dog_[name] = dog;
    return name_to_dog_.at(name);
}

void GameSession::UpdateTime(double time_delta)
{
    for (auto&[name, dog] : name_to_dog_) {
        
    }
}

}