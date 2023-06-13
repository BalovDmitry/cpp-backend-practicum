#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "../src/collision_detector.h"

// Напишите здесь тесты для функции collision_detector::FindGatherEvents

SCENARIO("Gathering events") {
    using namespace collision_detector;

    GIVEN("a provider") {
        auto provider = std::make_shared<ItemGathererProviderBase>();

        WHEN("there is no objects in provider") {
            THEN("empty result") {
                auto result = FindGatherEvents(*provider);
                CHECK(result.empty());
            }
        }
    }
}