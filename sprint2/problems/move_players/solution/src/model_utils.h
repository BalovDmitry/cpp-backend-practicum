#pragma once

namespace model {

struct Position {
    double x = 0.0;
    double y = 0.0;
};

struct Speed {
    double v_x = 0.0;
    double v_y = 0.0;
};

enum class Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST,
    NO_DIRECTION
};

}