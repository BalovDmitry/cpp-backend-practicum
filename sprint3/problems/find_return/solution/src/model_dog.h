#pragma once

#include "model_utils.h"

#include <string>

namespace model {

class Dog {
public:
    Dog(const std::string& name, Position position, double map_speed, Direction direction)
        : name_(name)
        , position_(position)
        , prev_position_(position)
        , map_speed_(map_speed)
        , direction_(direction) {}

    // Setters
    void SetDirection(Direction dir);
    void SetSpeedByDirection(Direction dir);
    void SetPosition(Position pos) { position_.x = pos.x; position_.y = pos.y; };
    void SetPrevPosition(Position pos) { prev_position_.x = pos.x; prev_position_.y = pos.y; };
    void SetSpeed(Speed speed) { speed_.v_x = speed.v_x; speed_.v_y = speed.v_y; }

    // Getters
    const Position& GetPosition() const { return position_; }
    const Position& GetPrevPosition() const { return prev_position_; }
    const Speed& GetSpeed() const { return speed_; }
    Direction GetDirection() const { return direction_; }
    const std::string& GetFullName() const { return name_; }
    std::string GetDirectionString() const;

private:
    const double map_speed_;
    std::string name_;

    Position position_;
    Position prev_position_;
    Speed speed_{0.0, 0.0};
    Direction direction_ = Direction::NO_DIRECTION;
};

}
