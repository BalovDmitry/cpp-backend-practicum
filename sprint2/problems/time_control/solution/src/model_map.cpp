#include "model_map.h"

#include <stdexcept>
#include <algorithm>

namespace model {

uint32_t Road::counter_ = 0;

RoadBoarders CalculateBoarders(const Road& road) {
    Position point_begin, point_end;

    point_begin.x = std::max(0.0, static_cast<double>(road.GetStart().x) - 0.4);
    point_begin.y = std::max(0.0, static_cast<double>(road.GetStart().y) - 0.4);
    point_end.x = static_cast<double>(road.GetEnd().x) + 0.4;
    point_end.y = static_cast<double>(road.GetEnd().y) + 0.4; 

    return { point_begin, point_end };
}

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

Position Map::CalculatePositionOnRoad(const Road& current_road, const Position& calculated_pos)
{
    Position result;

    if (current_road.IsHorizontal()) {
        double left_boarder = static_cast<double>(std::min(current_road.GetStart().x, current_road.GetEnd().x)) - 0.4;
        double right_boarder = static_cast<double>(std::max(current_road.GetStart().x, current_road.GetEnd().x)) + 0.4;
        if (calculated_pos.x < left_boarder) {
            result.x = left_boarder;
        } else if (calculated_pos.x > right_boarder) {
            result.x = right_boarder;
        } else {
            result.x = calculated_pos.x;
        }
        result.y = calculated_pos.y;
    } else {
        double upper_boarder = static_cast<double>(std::max(current_road.GetStart().y, current_road.GetEnd().y)) + 0.4;
        double lower_boarder = static_cast<double>(std::min(current_road.GetStart().y, current_road.GetEnd().y)) - 0.4;
        if (calculated_pos.y > upper_boarder) {
            result.y = upper_boarder;
        } else if (calculated_pos.y < lower_boarder) {
            result.y = lower_boarder;
        } else {
            result.y = calculated_pos.y;
        }
        result.x = calculated_pos.x;
    }
    return result;

}

std::pair<Position, Speed> Map::CalculatePositionAndSpeedOnRoad(const Road &current_road, const Position &calculated_pos, const Speed &initial_speed)
{
    Position result_pos;
    Speed result_speed = initial_speed;

    if (current_road.IsHorizontal()) {
        double left_boarder = static_cast<double>(std::min(current_road.GetStart().x, current_road.GetEnd().x)) - 0.4;
        double right_boarder = static_cast<double>(std::max(current_road.GetStart().x, current_road.GetEnd().x)) + 0.4;
        if (calculated_pos.x <= left_boarder) {
            result_pos.x = left_boarder;
            result_speed.v_x = 0;
        } else if (calculated_pos.x >= right_boarder) {
            result_pos.x = right_boarder;
            result_speed.v_x = 0;
        } else {
            result_pos.x = calculated_pos.x;
        }
        result_pos.y = calculated_pos.y;
    } else {
        double upper_boarder = static_cast<double>(std::max(current_road.GetStart().y, current_road.GetEnd().y)) + 0.4;
        double lower_boarder = static_cast<double>(std::min(current_road.GetStart().y, current_road.GetEnd().y)) - 0.4;
        if (calculated_pos.y >= upper_boarder) {
            result_pos.y = upper_boarder;
            result_speed.v_y = 0;
        } else if (calculated_pos.y <= lower_boarder) {
            result_pos.y = lower_boarder;
            result_speed.v_y = 0;
        } else {
            result_pos.y = calculated_pos.y;
        }
        result_pos.x = calculated_pos.x;
    }
    return std::make_pair<>(result_pos, result_speed);
}

std::optional<Road> Map::FindRoadByPosition(const Position &position)
{
    for (int i = 0; i < road_boraders_.size(); ++i) {
        if (road_boraders_[i].ContainPosition(position)) {
            return roads_[i];
        }
    }
    return std::optional<Road>();
}

std::optional<Road> Map::FindRoadByPositionExceptRoadId(const Position &position, int excepted_id)
{
    for (int i = 0; i < road_boraders_.size(); ++i) {
        if (i != excepted_id && road_boraders_[i].ContainPosition(position)) {
            return roads_[i];
        }
    }
    return std::optional<Road>();
}

} 
