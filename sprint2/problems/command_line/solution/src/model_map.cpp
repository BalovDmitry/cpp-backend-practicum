#include "model_map.h"

#include <stdexcept>
#include <algorithm>
#include <random>
#include <cstdlib>

namespace model {

RoadBoarders CalculateBoarders(const Road& road) {
    Position point_begin, point_end;

    point_begin.x = std::min(static_cast<double>(road.GetStart().x), static_cast<double>(road.GetEnd().x)) - 0.4;
    point_begin.y = std::min(static_cast<double>(road.GetStart().y) , static_cast<double>(road.GetEnd().y))  - 0.4;
    point_end.x = std::max(static_cast<double>(road.GetStart().x), static_cast<double>(road.GetEnd().x))  + 0.4;
    point_end.y = std::max(static_cast<double>(road.GetStart().y), static_cast<double>(road.GetEnd().y)) + 0.4; 

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

std::pair<Position, Speed> Map::CalculatePositionAndSpeedOnRoad(const Road &current_road, const Position &calculated_pos, const Speed &initial_speed) {
    Position result_pos;
    Speed result_speed = initial_speed;

    double left_boarder = static_cast<double>(std::min(current_road.GetStart().x, current_road.GetEnd().x)) - 0.4;
    double right_boarder = static_cast<double>(std::max(current_road.GetStart().x, current_road.GetEnd().x)) + 0.4;
    double upper_boarder = static_cast<double>(std::max(current_road.GetStart().y, current_road.GetEnd().y)) + 0.4;
    double lower_boarder = static_cast<double>(std::min(current_road.GetStart().y, current_road.GetEnd().y)) - 0.4;

    if (calculated_pos.x <= left_boarder) {
        result_pos.x = left_boarder;
        result_speed.v_x = 0;
    } else if (calculated_pos.x >= right_boarder) {
        result_pos.x = right_boarder;
        result_speed.v_x = 0;
    } else {
        result_pos.x = calculated_pos.x;
    }

    if (calculated_pos.y >= upper_boarder) {
        result_pos.y = upper_boarder;
        result_speed.v_y = 0;
    } else if (calculated_pos.y <= lower_boarder) {
        result_pos.y = lower_boarder;
        result_speed.v_y = 0;
    } else {
        result_pos.y = calculated_pos.y;
    }
    
    return std::make_pair<>(result_pos, result_speed);
}

std::pair<Position, Speed> Map::CalculatePositionAndSpeedOnRoad(const Road &current_road, const RoadBoarders &boarders, const Position &calculated_pos, const Speed &initial_speed) {
    Position result_pos;
    Speed result_speed = initial_speed;

    const double left_boarder = boarders.GetStartPoint().x;
    const double right_boarder = boarders.GetEndPoint().x;
    const double lower_boarder = boarders.GetStartPoint().y;
    const double upper_boarder = boarders.GetEndPoint().y;

    if (calculated_pos.x <= left_boarder) {
        result_pos.x = left_boarder;
        result_speed.v_x = 0;
    } else if (calculated_pos.x >= right_boarder) {
        result_pos.x = right_boarder;
        result_speed.v_x = 0;
    } else {
        result_pos.x = calculated_pos.x;
    }

    if (calculated_pos.y >= upper_boarder) {
        result_pos.y = upper_boarder;
        result_speed.v_y = 0;
    } else if (calculated_pos.y <= lower_boarder) {
        result_pos.y = lower_boarder;
        result_speed.v_y = 0;
    } else {
        result_pos.y = calculated_pos.y;
    }
    
    return std::make_pair<>(result_pos, result_speed);
}

std::optional<Road> Map::FindRoadByPosition(const Position &position) {
    for (int i = 0; i < road_boarders_.size(); ++i) {
        if (road_boarders_[i].ContainPosition(position)) {
            return roads_[i];
        }
    }
    return std::optional<Road>();
}

std::optional<Road> Map::FindRoadByPositionExceptRoadId(const Position &position, int excepted_id) {
    for (int i = 0; i < road_boarders_.size(); ++i) {
        if (i != excepted_id && road_boarders_[i].ContainPosition(position)) {
            return roads_[i];
        }
    }
    return std::optional<Road>();
}

Position Map::GetRandomPosition() const {
    int road_id = rand()%(roads_.size());
    auto boarders = road_boarders_[road_id];
    
    double random = ((double) rand()) / (double) RAND_MAX;
    double diff_x = boarders.GetEndPoint().x - boarders.GetStartPoint().x;
    double diff_y = boarders.GetEndPoint().y - boarders.GetStartPoint().y;

    double r_x = random * diff_x;
    double r_y = random * diff_y;

    double pos_x = boarders.GetStartPoint().x + r_x;
    double pos_y = boarders.GetStartPoint().y + r_y;

    return { pos_x, pos_y };
}

} 
