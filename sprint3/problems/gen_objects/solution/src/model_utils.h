#pragma once

namespace model {

const static double DEFAULT_DOG_SPEED = 1.0;
const static double ROAD_WIDTH = 0.4;

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

double CalculateDistance(const Position& pos1, const Position& pos2);
double CalculateAbsSpeed(const Speed& speed);

}