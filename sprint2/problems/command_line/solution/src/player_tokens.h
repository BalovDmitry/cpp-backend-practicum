#pragma once

#include <string>
#include <random>
#include <unordered_map>
#include <vector>

#include "model_player.h"
#include "tagged.h"

namespace model {

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
    const std::vector<Player>& GetPlayers() const;

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

}