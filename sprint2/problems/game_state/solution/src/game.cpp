#include "game.h"
#include "logger.h"
#include "request_handler_helper.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace model {

using namespace std::literals;

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

const Game::Maps& Game::GetMaps() const noexcept {
    return maps_;
}

const Map* Game::FindMap(const Map::Id& id) const noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return &maps_.at(it->second);
    }
    return nullptr;
}

Token Game::JoinGame(const std::string &name, const Map::Id &id)
{
    auto map = FindMap(id);
    if (!map) {
        throw std::invalid_argument(std::string(http_handler::ErrorMessages::INVALID_ARGUMENT_MAP));
    }

    if (!name_to_id_.contains(name)) {
        const auto player_id = current_id_++;
        name_to_id_[name] = player_id;
        map_id_to_player_id_[id].insert(player_id);
        auto session = FindSession(id);
        auto dog = session->AddDog({0.0, 0.0}, name);

        return player_tokens_.AddPlayer({ name, player_id, session, dog });
    } else {
        return player_tokens_.FindTokenByPlayerId(name_to_id_.at(name));
    }
}

const Player &Game::FindPlayerByToken(Token token)
{
    return player_tokens_.FindPlayerByToken(token);
}

const Player &Game::FindPlayerById(uint32_t id)
{
    return player_tokens_.FindPlayerById(id);
}

SessionPtr Game::FindSession(Map::Id id)
{
    if (!map_id_to_session_.contains(id)) {
        if (auto map = FindMap(id)) {
            map_id_to_session_[id] = std::make_shared<GameSession>(const_cast<Map&>(*map));
        } else {
            throw std::invalid_argument(std::string(http_handler::ErrorMessages::INVALID_ARGUMENT_MAP));
        }
    }
    return map_id_to_session_.at(id);
}

const std::unordered_set<uint32_t> &Game::GetPlayersOnMap(Map::Id id)
{
    if (!map_id_to_player_id_.contains(id)) {
        throw std::invalid_argument(std::string(http_handler::ErrorMessages::INVALID_ARGUMENT_MAP));
    }
    return map_id_to_player_id_.at(id);
}

}  // namespace model
