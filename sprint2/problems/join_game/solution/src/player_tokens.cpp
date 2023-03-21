#include "player_tokens.h"
#include "logger.h"

#include <iostream>
#include <sstream>
#include <iomanip>

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