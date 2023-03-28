#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <memory>

#include "tagged.h"

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

private:
    Point start_;
    Point end_;
};

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

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Dog {
public:
    Dog(const std::string& name) 
    {
        name_ = name + "_" + std::to_string(Dog::id);
        ++Dog::id;
    }

    const std::string& GetFullName() const { return name_; }

private:
    static uint32_t id;
    std::string name_;
};

class GameSession {
public:
    GameSession(model::Map& map)
        : map_(map) {}

    GameSession(const GameSession& session) = default;
    GameSession& operator=(const GameSession& session) {
        dogs_ = session.dogs_;
        map_ = session.map_;
        return *this;
    }
    const auto& GetMapId() const { return map_.GetId(); }

private:
    std::unordered_map<uint32_t, Dog> dogs_;
    model::Map& map_;
};

class Player {
public:
    Player(const std::string& name, model::Map& map, uint32_t id)
        : name_(name)
        , session_(map) 
        , dog_(name) 
        , id_(id) {}

    const uint32_t GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    const auto& GetMapId() const { return session_.GetMapId(); }

private:
    u_int32_t id_ = 0;
    GameSession session_;
    std::string name_;
    Dog dog_;
};

namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;
static std::random_device RandomDevice;

class PlayerTokens {
public:
    Token AddPlayer(Player&& player);
    const Player& FindPlayerByToken(const Token& token) const;
    const Player& FindPlayerById(uint32_t id);
    const Token& FindTokenByPlayerId(uint32_t id);

private:
    std::unordered_map<Token, uint32_t, util::TaggedHasher<Token>> tokenToPlayer_;
    std::vector<Player> players_;
    std::vector<Token> tokens_;

    std::mt19937_64 generator1_{[] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(RandomDevice);
    }()};
    std::mt19937_64 generator2_{[] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(RandomDevice);
    }()};
};


using SessionPtr = std::shared_ptr<GameSession>;

class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    Token JoinGame(const std::string &name, const Map::Id& id);
    const Player& FindPlayerByToken(Token token);
    const Player& FindPlayerById(uint32_t id);
    SessionPtr FindSession(Map::Id id);
    const std::unordered_set<uint32_t>& GetPlayersOnMap(Map::Id id);
    
private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    using MapIdToPlayers = std::unordered_map<Map::Id, std::unordered_set<uint32_t> , MapIdHasher>;
    using MapIdToSession = std::unordered_map<Map::Id, SessionPtr , MapIdHasher>;

    uint32_t current_id_ = 0;

    std::vector<Map> maps_;
    //std::vector<GameSession> sessions_;

    std::unordered_map<std::string, uint32_t> name_to_id_;

    MapIdToIndex map_id_to_index_;
    MapIdToPlayers map_id_to_player_id_;
    MapIdToSession map_id_to_session_;
    PlayerTokens player_tokens_;
};

}  // namespace model
