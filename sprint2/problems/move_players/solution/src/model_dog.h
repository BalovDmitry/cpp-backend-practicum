#pragma once

#include "model_utils.h"

#include <string>

namespace model {

class Dog {
public:
    Dog(const std::string& name)
        : name_(name) {}

    // Setters
    void SetPosition(Position pos) { position_.x = pos.x; position_.y = pos.y; };
    void SetSpeed(Speed speed) { speed_.v_x = speed.v_x; speed_.v_y = speed.v_y; }
    void SetSpeed(double speed) { speed_.v_x = speed; speed_.v_y = speed; }
    void SetDirection(Direction dir) { if (dir != Direction::NO_DIRECTION) direction_ = dir; }

    // Getters
    const Position& GetPosition() const { return position_; }
    const Speed& GetSpeed() const { return speed_; }
    Direction GetDirection() const { return direction_; }
    const std::string& GetFullName() const { return name_; }
    
    std::string GetDirectionString() const {
        switch (direction_) {
            case Direction::EAST: return "R";
            case Direction::WEST: return "L";
            case Direction::NORTH: return "U";
            case Direction::SOUTH: return "D";
            case Direction::NO_DIRECTION: return "";
        }
    }

private:
    Position position_;
    Speed speed_;
    Direction direction_ = Direction::NORTH;
    std::string name_;
};

}
