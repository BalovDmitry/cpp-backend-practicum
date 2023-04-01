#include "game_session.h"

#include <algorithm>
#include <utility>
#include <iostream>

namespace model {

DogPtr GameSession::AddDog(Position spawn_point, const std::string& name) {
    auto dog = std::make_shared<Dog>(name, spawn_point, map_.GetSpeed(), Direction::NORTH);
    name_to_dog_[name] = dog;
    return name_to_dog_.at(name);
}

void GameSession::UpdateTime(double time_delta)
{
    for (auto&[name, dog] : name_to_dog_) {
        UpdateDogPosition(dog, time_delta);
    }
}

void GameSession::UpdateDogPosition(DogPtr dog, double time_delta)
{
    Position calculated_pos;

    std::cout << "Initial position: " << "x: " << dog->GetPosition().x << ", y: " << dog->GetPosition().y << std::endl;

    Position calculated_pos_on_current_road = dog->GetPosition();
    Speed calculated_speed_on_current_road;

    Position calculated_pos_on_other_road = dog->GetPosition();
    Speed calculated_speed_on_other_road;

    calculated_pos.x = dog->GetPosition().x + dog->GetSpeed().v_x * time_delta / 1000;
    calculated_pos.y = dog->GetPosition().y + dog->GetSpeed().v_y * time_delta / 1000;

    std::cout << "Calculated position: " << "x: " << calculated_pos.x << ", y: " << calculated_pos.y << std::endl;

    auto current_road = map_.FindRoadByPosition(dog->GetPosition());
    if (current_road.has_value()) {
        auto result_on_current_road = map_.CalculatePositionAndSpeedOnRoad(current_road.value(), calculated_pos, dog->GetSpeed());
        calculated_pos_on_current_road = result_on_current_road.first;
        calculated_speed_on_current_road = result_on_current_road.second;

        auto other_road = map_.FindRoadByPositionExceptRoadId(dog->GetPosition(), current_road.value().GetId());
        if (other_road.has_value()) {
            auto result_on_other_road = map_.CalculatePositionAndSpeedOnRoad(other_road.value(), calculated_pos, dog->GetSpeed());
            calculated_pos_on_other_road = result_on_other_road.first;
            calculated_speed_on_other_road = result_on_other_road.second;
        }
    }

    auto distance_on_current_road = CalculateDistance(calculated_pos_on_current_road, dog->GetPosition());
    auto distance_on_other_road = CalculateDistance(calculated_pos_on_other_road, dog->GetPosition());
    
    if (distance_on_current_road < distance_on_other_road) {
        dog->SetPosition(calculated_pos_on_other_road);
        dog->SetSpeed(calculated_speed_on_other_road);
    } else if (distance_on_current_road > distance_on_other_road) {
        dog->SetPosition(calculated_pos_on_current_road);
        dog->SetSpeed(calculated_speed_on_current_road);
    } else {
        if (CalculateAbsSpeed(calculated_speed_on_current_road) > CalculateAbsSpeed(calculated_speed_on_other_road)) {
            dog->SetPosition(calculated_pos_on_current_road);
            dog->SetSpeed(calculated_speed_on_current_road);
        } else {
            dog->SetPosition(calculated_pos_on_other_road);
            dog->SetSpeed(calculated_speed_on_other_road);
        }
    }
        //throw std::invalid_argument("Distances are equal");
    //}
    // } else {
    //     dog->SetPosition(calculated_pos_on_current_road);
    //     double max_speed_v_x = std::max(std::abs(calculated_speed_on_current_road.v_x), std::abs(calculated_speed_on_other_road.v_x));
    //     double max_speed_v_y = std::max(std::abs(calculated_speed_on_current_road.v_y), std::abs(calculated_speed_on_other_road.v_y));
    //     dog->SetSpeed( { max_speed_v_x, max_speed_v_y } );
    // }

    std::cout << "Final position: " << "x: " << dog->GetPosition().x << ", y: " << dog->GetPosition().y << std::endl;

}

}