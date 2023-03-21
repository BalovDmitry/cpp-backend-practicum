#pragma once

#include "dog.h"
#include "model.h"
#include <unordered_map>

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
private:
    std::unordered_map<uint32_t, Dog> dogs_;
    model::Map& map_;
};