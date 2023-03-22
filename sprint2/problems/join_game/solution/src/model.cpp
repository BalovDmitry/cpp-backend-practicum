#include "model.h"
#include "logger.h"

#include <stdexcept>
#include <sstream>

namespace model {
using namespace std::literals;

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

Token PlayerTokens::AddPlayer(const Player& player) {
    auto first_part = generator1_();
    auto second_part = generator2_();

    std::stringstream buf;
    buf << std::hex << first_part << second_part;

    auto result = Token(buf.str());
    tokenToPlayer_.emplace(result, player);
    //tokenToPlayer_[result] = player;
    return result;
}

Player PlayerTokens::FindPlayerBy(const Token& token) {
    if (tokenToPlayer_.contains(token)) {
        logger::LogJsonAndMessage(boost::json::object( {{"token", *token}}) , "player is found");
        return tokenToPlayer_.at(token);
    } else {
        logger::LogJsonAndMessage(boost::json::object( {{"token", *token}}) , "player is not found");
        throw std::runtime_error("Invalid argument: player not found!");
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

}  // namespace model
