#pragma once

#include <string>

namespace model {

struct Position {
    double x = 0.0;
    double y = 0.0;
};

struct Velocity {
    double v_x = 0.0;
    double v_y = 0.0;
};

enum class Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST
};

class Dog {
public:
    Dog(const std::string& name)
        : name_(name) {}

    // Setters
    void SetPosition(Position pos) { position_.x = pos.x; position_.y = pos.y; };
    void SetVelocity(Velocity vel) { velocity_.v_x = vel.v_x; velocity_.v_y = vel.v_y; }
    void SetDirection(Direction dir) { direction_ = dir; }

    // Getters
    const Position& GetPosition() const { return position_; }
    const Velocity& GetVelocity() const { return velocity_; }
    Direction GetDirection() const { return direction_; }
    const std::string& GetFullName() const { return name_; }
    
    std::string GetDirectionString() const {
        switch (direction_) {
            case Direction::EAST: return "R";
            case Direction::WEST: return "L";
            case Direction::NORTH: return "U";
            case Direction::SOUTH: return "D";
        }
    }

private:
    Position position_;
    Velocity velocity_;
    Direction direction_ = Direction::NORTH;
    std::string name_;
};

}
