#pragma once

#include <string>

#include "game_session.h"
#include "dog.h"

class Player {
public:

    Player(const std::string& name)
    //, const GameSession& session)
        : name_(name) {}
        //, session_(session) {}
private:
    //GameSession session_;
    std::string name_;
    Dog dog_;
};