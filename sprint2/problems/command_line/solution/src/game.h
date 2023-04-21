#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>

#include "model_dog.h"
#include "model_map.h"
#include "game_session.h"
#include "model_player.h"
#include "player_tokens.h"

#include "tagged.h"

namespace model {

class Game {
public:
    using Maps = std::vector<Map>;
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    using MapIdToSession = std::unordered_map<Map::Id, SessionPtr , MapIdHasher>;

    void AddMap(Map map);
    const Map* FindMap(const Map::Id& id) const noexcept;

    // Change game state
    Token JoinGame(const std::string &name, const Map::Id& id, bool randomize_spawn_point = false);
    const Player& FindPlayerByToken(Token token);
    const Player& FindPlayerById(uint32_t id);
    SessionPtr FindSession(Map::Id id);
    
    // Getters
    const Maps& GetMaps() const noexcept;
    const auto& GetPlayers() const { return player_tokens_.GetPlayers(); }
    auto& GetMapToSession() { return map_id_to_session_; }
    
private:
    uint32_t current_id_ = 0;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    MapIdToSession map_id_to_session_;
    PlayerTokens player_tokens_;
};

}  // namespace model
