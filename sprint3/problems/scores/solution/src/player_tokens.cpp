#include "player_tokens.h"
#include "game_server_exceptions.h"
//#include "request_handler_helper.h"

#include <sstream>
#include <iostream>

namespace model {

const static uint8_t TOKEN_SIZE = 32;

Token PlayerTokens::AddPlayer(Player&& player) {
    std::stringstream buf;

    auto first_part = generator1_();
    auto second_part = generator2_();
    buf << std::hex << first_part << second_part;

    std::string token_string;
    token_string += buf.str();
    while (token_string.size() < TOKEN_SIZE) {
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
        throw server_exceptions::InvalidTokenException("Invalid token size");
    }

    if (tokenToPlayer_.contains(token)) {
        auto id = tokenToPlayer_.at(token);
        return players_[id];
    } else {
        throw server_exceptions::UnknownTokenException("Player token has not been found");
    }
}

const Player &PlayerTokens::FindPlayerById(uint32_t id) {
    if (id >= tokens_.size()) {
        throw server_exceptions::InvalidArgumentException("Invalid player id");
    }
    auto token = tokens_[id];
    return FindPlayerByToken(token);
}

const Token &PlayerTokens::FindTokenByPlayerId(uint32_t id) {
    if (id >= tokens_.size()) {
        throw server_exceptions::InvalidArgumentException("Invalid player id");
    }
    return tokens_[id];
}

const std::vector<Player> &PlayerTokens::GetPlayers() const {
    return players_;
}

}