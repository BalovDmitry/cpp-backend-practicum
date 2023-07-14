#include "model_utils.h"

#include <cmath>
#include <algorithm>

double model::CalculateDistance(const Position &pos1, const Position &pos2)
{
    return std::sqrt(std::pow(pos1.x - pos2.x, 2) + std::pow(pos1.y - pos2.y, 2));
}

double model::CalculateAbsSpeed(const Speed &speed)
{
    return std::sqrt(std::pow(speed.v_x, 2) + std::pow(speed.v_y, 2));
}