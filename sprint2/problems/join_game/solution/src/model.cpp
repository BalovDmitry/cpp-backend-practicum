#include "model.h"
#include "logger.h"
#include "request_handler_helper.h"

#include <stdexcept>
#include <sstream>

namespace model {
using namespace std::literals;

uint32_t Dog::id = 0;

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

Token PlayerTokens::AddPlayer(Player&& player) {
    std::stringstream buf;
    while (buf.str().length() != 32) {
        auto first_part = generator1_();
        auto second_part = generator2_();
        buf << '0';
        buf << std::hex << first_part << second_part;
    }

    auto result = Token(buf.str());

    tokenToPlayer_.emplace(result, player.GetId());
    players_.emplace_back(std::move(player));
    tokens_.push_back(result);

    return result;
}

const Player& PlayerTokens::FindPlayerByToken(const Token& token) const {
    if (tokenToPlayer_.contains(token)) {
        auto id = tokenToPlayer_.at(token);
        return players_[id];
    } else {
        throw std::invalid_argument(std::string(http_handler::ErrorMessages::UNKNOWN_TOKEN));
    }
}

const Player &PlayerTokens::FindPlayerById(uint32_t id)
{
    if (id >= tokens_.size()) {
        throw std::invalid_argument("Invalid palyer id!");
    }
    auto token = tokens_[id];
    return FindPlayerByToken(token);
}

const Token &PlayerTokens::FindTokenByPlayerId(uint32_t id)
{
    if (id >= tokens_.size()) {
        throw std::invalid_argument("Invalid palyer id!");
    }
    return tokens_[id];
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
        return player_tokens_.AddPlayer({name, const_cast<model::Map&>(*map), player_id});
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

const std::unordered_set<uint32_t> &Game::GetPlayersOnMap(Map::Id id)
{
    if (!map_id_to_player_id_.contains(id)) {
        throw std::invalid_argument(std::string(http_handler::ErrorMessages::INVALID_ARGUMENT_MAP));
    }
    return map_id_to_player_id_.at(id);
}

}  // namespace model
