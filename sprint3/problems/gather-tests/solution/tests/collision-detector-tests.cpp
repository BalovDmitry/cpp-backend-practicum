#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "../src/collision_detector.h"

// Напишите здесь тесты для функции collision_detector::FindGatherEvents


SCENARIO("Gathering events") {

    using namespace collision_detector;
    class ItemGathererProviderBase : public ItemGathererProvider {
    public:
        size_t ItemsCount() const override {
            return items_.size();
        }

        Item GetItem(size_t idx) const override {
            return items_.at(idx);
        }

        size_t GatherersCount() const override {
            return gatherers_.size();
        }

        Gatherer GetGatherer(size_t idx) const override {
            return gatherers_.at(idx);
        }

    public:
        void PushItem(Item&& item) {
            items_.emplace_back(item);
        }

        void PushGatherer(Gatherer&& gatherer) {
            gatherers_.emplace_back(gatherer);
        }

    private:
        std::vector<Item> items_;
        std::vector<Gatherer> gatherers_;
    };


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