#include "player_tokens.h"
#include "request_handler_helper.h"

#include <sstream>
#include <iostream>

namespace model {

Token PlayerTokens::AddPlayer(Player&& player) {
    std::stringstream buf;

    auto first_part = generator1_();
    auto second_part = generator2_();
    buf << std::hex << first_part << second_part;

    std::string token_string;
    token_string += buf.str();
    if (token_string.size() == 30) {
        token_string.insert(0, "00");
    } else if (token_string.size() == 31) {
        token_string.insert(0, "0");
    }

    auto result = Token(std::move(token_string));
    
    tokenToPlayer_.emplace(result, player.GetId());
    players_.emplace_back(std::move(player));
    tokens_.push_back(result);
    return result;
}

const Player& PlayerTokens::FindPlayerByToken(const Token& token) const {
    if ((*token).size() != 32) {
        throw http_handler::InvalidTokenException("Invalid token size");
        //throw std::invalid_argument(std::string(http_handler::ErrorMessages::INVALID_TOKEN));
    }

    if (tokenToPlayer_.contains(token)) {
        auto id = tokenToPlayer_.at(token);
        return players_[id];
    } else {
        throw http_handler::UnknownTokenException("Player token has not been found");
        //throw std::invalid_argument(std::string(http_handler::ErrorMessages::UNKNOWN_TOKEN));
    }
}

const Player &PlayerTokens::FindPlayerById(uint32_t id) {
    if (id >= tokens_.size()) {
        throw http_handler::InvalidArgumentException("Invalid player id");
        //throw std::invalid_argument("Invalid palyer id!");
    }
    auto token = tokens_[id];
    return FindPlayerByToken(token);
}

const Token &PlayerTokens::FindTokenByPlayerId(uint32_t id) {
    if (id >= tokens_.size()) {
        throw http_handler::InvalidArgumentException("Invalid player id");
        //throw std::invalid_argument("Invalid palyer id!");
    }
    return tokens_[id];
}

const std::vector<Player> &PlayerTokens::GetPlayers() const {
    return players_;
}

}