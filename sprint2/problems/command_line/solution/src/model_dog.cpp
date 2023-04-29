#include "model_dog.h"

#include <stdexcept>

namespace model {
void Dog::SetDirection(Direction dir) {
    if (dir != Direction::NO_DIRECTION) {
        direction_ = dir; 
    }
}

std::string Dog::GetDirectionString() const {
    switch (direction_) {
        case Direction::EAST: return "R";
        case Direction::WEST: return "L";
        case Direction::NORTH: return "U";
        case Direction::SOUTH: return "D";
        case Direction::NO_DIRECTION:
        default: return "";
    }
}

void Dog::SetSpeedByDirection(Direction dir) {
    switch (dir) {
        case Direction::EAST: {
            SetSpeed({map_speed_, 0});
            break;
        }

        case Direction::WEST: {
            SetSpeed({(-1) * map_speed_, 0});
            break;
        }

        case Direction::NORTH: {
            SetSpeed({0, (-1) * map_speed_});
            break;
        }

        case Direction::SOUTH: {
            SetSpeed({0, map_speed_});
            break;
        }

        case Direction::NO_DIRECTION: {
            SetSpeed({0, 0});
            break;
        }

        default: {
            break;
        }
    }
}

}