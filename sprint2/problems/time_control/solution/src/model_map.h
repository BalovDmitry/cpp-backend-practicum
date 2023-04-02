#pragma once

#include "tagged.h"
#include "model_utils.h"

#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <iostream>

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class RoadBoarders {
public:
    RoadBoarders(Position point_begin, Position point_end)
        : point_begin_(point_begin)
        , point_end_(point_end) {}

    RoadBoarders(const RoadBoarders& boarders) {
        point_begin_.x = boarders.point_begin_.x;
        point_begin_.y = boarders.point_begin_.y;
        point_end_.x = boarders.point_end_.x;
        point_end_.y = boarders.point_end_.y;
    }

    RoadBoarders& operator=(const RoadBoarders& boarders) {
        point_begin_.x = boarders.point_begin_.x;
        point_begin_.y = boarders.point_begin_.y;
        point_end_.x = boarders.point_end_.x;
        point_end_.y = boarders.point_end_.y;
        return *this;
    }

    const Position& GetStartPoint() const {
        return point_begin_;
    }

    const Position& GetEndPoint() const {
        return point_end_;
    }

    bool ContainPosition(const Position& position) {
        return (position.x >= point_begin_.x && position.x <= point_end_.x) 
                && (position.y >= point_begin_.y && position.y <= point_end_.y);
    }
private:
    Position point_begin_;
    Position point_end_;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

    uint32_t GetId() const noexcept {
        return id_;
    }

    void SetId(uint32_t id) {
        id_ = id;
    } 

private:
    Point start_;
    Point end_;
    uint32_t id_ = 0;
};

RoadBoarders CalculateBoarders(const Road& road);


class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Boarders = std::vector<RoadBoarders>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    const Boarders& GetRoadBoarders() const noexcept {
        return road_boarders_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
        roads_.back().SetId(roads_.size() - 1);
        road_boarders_.emplace_back(CalculateBoarders(road));
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void SetSpeed(double speed) {
        speed_ = speed;
    }
 
    double GetSpeed() const {
        return speed_;
    }

    void AddOffice(Office office);
    std::pair<Position, Speed> CalculatePositionAndSpeedOnRoad(const Road& current_road, const Position& calculated_pos, const Speed& initial_speed);
    std::optional<Road> FindRoadByPosition(const Position& position);
    std::optional<Road> FindRoadByPositionExceptRoadId(const Position& position, int excepted_id);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Boarders road_boarders_;
    Buildings buildings_;
    double speed_{1.0};

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;

};

}