#include "game_session.h"
#include "json_helper.h"

#include <algorithm>
#include <utility>
#include <iostream>
#include <random>

namespace model {

std::optional<uint32_t> GameSession::GetPlayerIdByName(const std::string &name) {
    if (HasPlayerWithName(name)) {
        return name_to_id_.at(name);
    }
    return std::nullopt;
}

void GameSession::SetLootGeneratorData(double base_interval, double probability) {
    using namespace std::chrono_literals;
    //std::chrono::milliseconds interval = static_cast<long long>(base_interval) * 1ms;
    loot_generator_ = std::make_shared<loot_gen::LootGenerator>(static_cast<long long>(base_interval) * 1ms, probability);
}

DogPtr GameSession::AddDog(Position spawn_point, const std::string& name, uint32_t id) {
    auto dog = std::make_shared<Dog>(name, spawn_point, GetMapSpeed(), Direction::NORTH);
    name_to_dog_[name] = dog;
    name_to_id_[name] = id;
    return name_to_dog_.at(name);
}

void GameSession::UpdateTime(std::chrono::milliseconds delta) {
    for (auto&[name, dog] : name_to_dog_) {
        UpdateDogPosition(dog, delta);
    }
    UpdateLostObjects(delta);
}

void GameSession::UpdateLostObjects(std::chrono::milliseconds delta) {
    loot_count_ = loot_generator_->Generate(delta, loot_count_, name_to_dog_.size());
    for (int i = 0; i < loot_count_; ++i) {
        loot_object_[std::to_string(i)] = json_helper::CreateLostObjectValue(rand() % loot_size_, map_.GetRandomPosition());
    }
}

void GameSession::UpdateDogPosition(DogPtr dog, std::chrono::milliseconds delta) {
    static const int TIME_RATE = 1000;

    Position calculated_pos;

    Position calculated_pos_on_current_road = dog->GetPosition();
    Speed calculated_speed_on_current_road;

    Position calculated_pos_on_other_road = dog->GetPosition();
    Speed calculated_speed_on_other_road;

    calculated_pos.x = dog->GetPosition().x + dog->GetSpeed().v_x * delta.count() / TIME_RATE;
    calculated_pos.y = dog->GetPosition().y + dog->GetSpeed().v_y * delta.count() / TIME_RATE;

    auto current_road = map_.FindRoadByPosition(dog->GetPosition());
    if (current_road.has_value()) {
        auto result_on_current_road = map_.CalculatePositionAndSpeedOnRoad(current_road.value(), map_.GetRoadBoarders().at(current_road.value().GetId()), calculated_pos, dog->GetSpeed());
        calculated_pos_on_current_road = result_on_current_road.first;
        calculated_speed_on_current_road = result_on_current_road.second;

        auto other_road = map_.FindRoadByPositionExceptRoadId(dog->GetPosition(), current_road.value().GetId());
        if (other_road.has_value()) {
            auto result_on_other_road = map_.CalculatePositionAndSpeedOnRoad(other_road.value(), map_.GetRoadBoarders().at(other_road.value().GetId()), calculated_pos, dog->GetSpeed());
            calculated_pos_on_other_road = result_on_other_road.first;
            calculated_speed_on_other_road = result_on_other_road.second;
        }
    }

    auto distance_on_current_road = CalculateDistance(calculated_pos_on_current_road, dog->GetPosition());
    auto distance_on_other_road = CalculateDistance(calculated_pos_on_other_road, dog->GetPosition());
    
    if (distance_on_current_road > distance_on_other_road) {
        dog->SetPosition(calculated_pos_on_current_road);
        dog->SetSpeed(calculated_speed_on_current_road);
    } else if (distance_on_current_road < distance_on_other_road) {
        dog->SetPosition(calculated_pos_on_other_road);
        dog->SetSpeed(calculated_speed_on_other_road);
    } else {
        if (CalculateAbsSpeed(calculated_speed_on_current_road) > CalculateAbsSpeed(calculated_speed_on_other_road)) {
            dog->SetPosition(calculated_pos_on_current_road);
            dog->SetSpeed(calculated_speed_on_current_road);
        } else if (CalculateAbsSpeed(calculated_speed_on_current_road) < CalculateAbsSpeed(calculated_speed_on_other_road)) {
            dog->SetPosition(calculated_pos_on_other_road);
            dog->SetSpeed(calculated_speed_on_other_road);
        } else {
            dog->SetPosition(calculated_pos_on_other_road);
            dog->SetSpeed(calculated_speed_on_other_road);
        }
    }
}

}
