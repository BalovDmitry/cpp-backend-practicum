#include <catch2/catch_test_macros.hpp>

#include "../src/game.h"

using namespace std::literals;

SCENARIO("Lost object generation") { 

    GIVEN("session instance") {
        // Constants
        const model::Map::Id MAP_ID_1("1");
        const std::string MAP_NAME_1 = "map1";
        constexpr std::chrono::milliseconds DELTA = 10000ms;
        constexpr unsigned LOOT_SIZE = 2;
        const std::string PLAYER_NAME_1 = "Player 1";
        const std::string PLAYER_NAME_2 = "Player 2";
        const std::string PLAYER_NAME_3 = "Player 3";
        constexpr unsigned PLAYER_ID_1 = 1;
        constexpr unsigned PLAYER_ID_2 = 2;
        constexpr unsigned PLAYER_ID_3 = 3;

        // Game objects
        model::Map map_1(MAP_ID_1, MAP_NAME_1);

        // Init extra data instance
        model::ExtraData::GetInstance().SetLootGeneratorData(100, 0.5);

        // Create session with LOOT_SIZE
        model::SessionPtr session = std::make_shared<model::GameSession>(map_1, LOOT_SIZE);

        WHEN("there is no players on map") {
            THEN("before time updating number of lost objects is equal to zero") {
                CHECK(session->GetPlayers().size() == 0);
                REQUIRE(session->GetLootCount() == 0);
            }

            THEN("after time updating number of lost objects is equal to zero") {
                session->UpdateTime(DELTA);
                CHECK(session->GetPlayers().size() == 0);
                REQUIRE(session->GetLootCount() == 0);
            }
        }

        WHEN("there is 1 player on map") {
            session->AddDog({0, 0}, PLAYER_NAME_1, PLAYER_ID_1);
            
            THEN("before time updating number of lost objects is equal to zero") {
                CHECK(session->GetPlayers().size() == 1);
                REQUIRE(session->GetLootCount() == 0);
            }
            
            THEN("after time updating number of lost objects is not equal to zero") {
                session->UpdateTime(DELTA);
                CHECK(session->GetPlayers().size() == 1);
                REQUIRE(session->GetLootCount() > 0);
                REQUIRE(session->GetLootCount() <= 1);
            }
        }

        WHEN("there are 3 players on map") {
            session->AddDog({0, 0}, PLAYER_NAME_1, PLAYER_ID_1);
            session->AddDog({0, 0}, PLAYER_NAME_2, PLAYER_ID_2);
            session->AddDog({0, 0}, PLAYER_NAME_3, PLAYER_ID_3);
            
            THEN("before time updating number of lost objects is equal to zero") {
                CHECK(session->GetPlayers().size() == 3);
                REQUIRE(session->GetLootCount() == 0);
            }
            
            THEN("after time updating number of lost objects is not equal to zero") {
                session->UpdateTime(DELTA);
                CHECK(session->GetPlayers().size() == 3);
                REQUIRE(session->GetLootCount() > 0);
                REQUIRE(session->GetLootCount() <= 3);
            }
        }
    }

}