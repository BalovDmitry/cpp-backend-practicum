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
    loot_generator_ = std::make_shared<loot_gen::LootGenerator>(static_cast<long long>(base_interval) * 1ms, probability);
}

DogPtr GameSession::AddDog(Position spawn_point, const std::string& name, uint32_t id) {
    auto dog = std::make_shared<Dog>(name, spawn_point, GetMapSpeed(), Direction::NORTH);
    name_to_id_[name] = id;
    id_to_dog_[id] = dog;
    return id_to_dog_.at(id);
}

void GameSession::UpdateTime(std::chrono::milliseconds delta) {
    for (auto&[id, dog] : id_to_dog_) {
        UpdateDogPosition(dog, delta);
    }
    UpdateLostObjects(delta);
    UpdateLootProvider();
    UpdateCollisions();
}

void GameSession::UpdateLostObjects(std::chrono::milliseconds delta) {
    TryGenerateLoot(delta);
}

void GameSession::TryGenerateLoot(std::chrono::milliseconds delta) {
    auto current_loot_count = available_loot_items_.size();
    auto calculated_loot_count = loot_generator_->Generate(delta, current_loot_count, id_to_dog_.size());
    if (calculated_loot_count > current_loot_count) {
        for (int i = current_loot_count; i < calculated_loot_count; ++i) {
            available_loot_items_[loot_id_++] = LootItem(rand() % loot_size_, map_.GetRandomPosition());
        }
    }
}

void GameSession::UpdateLootProvider() {
    // Clear previous data
    loot_provider_.Clear();
    
    // Add new data
    for (const auto&[id, dog] : id_to_dog_) {
        loot_provider_.PushGatherer(collision_detector::Gatherer(dog->GetPrevPosition(), dog->GetPosition(), PLAYER_WIDTH, id));
    }

    for (const auto&[id, item] : available_loot_items_) {
        loot_provider_.PushItem(collision_detector::Item(item.position, LOOT_WIDTH, id));
    }
}

void GameSession::UpdateCollisions() {
    auto gather_events = collision_detector::FindGatherEvents(loot_provider_);

    // For debug only
    // std::cout << "Available items: " << std::endl;
    // for (const auto& item : available_loot_items_) {
    //     std::cout << "id: " << item.first << ", x: " << item.second.position.x << ", y: " << item.second.position.y << std::endl;
    // }

    for (const auto& e : gather_events) {
        auto gatherer_id = e.gatherer_id;
        auto item_id = e.item_id;
        auto& dog = id_to_dog_.at(gatherer_id);
        if (dog->GetBagContent().size() < map_.GetBagCapacity()) {
            if (available_loot_items_.contains(item_id)) {
                dog->AddLootIntoBag(item_id, available_loot_items_.at(item_id));
                available_loot_items_.erase(item_id);
            }
        }
    }
}

void GameSession::UpdateDogPosition(DogPtr dog, std::chrono::milliseconds delta) {
    static const int TIME_RATE = 1000;

    dog->SetPrevPosition(dog->GetPosition());

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
