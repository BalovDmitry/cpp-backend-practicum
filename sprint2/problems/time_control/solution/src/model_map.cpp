#include "model_map.h"

#include <stdexcept>
#include <algorithm>

namespace model {

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

std::optional<Road> Map::FindRoadByPosition(const Position &position)
{
    for (int i = 0; i < road_boraders_.size(); ++i) {
        if (road_boraders_[i].ContainPosition(position)) {
            return roads_[i];
        }
    }
    return std::optional<Road>();
}

} 
