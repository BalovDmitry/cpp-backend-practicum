#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <memory>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>

#include "../src/collision_detector.h"

using Catch::Matchers::WithinAbs;
using namespace collision_detector;
using namespace std::literals;

namespace Catch {
template<>
struct StringMaker<collision_detector::GatheringEvent> {
  static std::string convert(collision_detector::GatheringEvent const& value) {
      std::ostringstream tmp;
      tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

      return tmp.str();
  }
};
} 

// Matchers block

const double AVAILABLE_ERROR = 1e-10;

auto are_equal_events = [](const GatheringEvent& lhs, const GatheringEvent& rhs) {
    return lhs.item_id == rhs.item_id 
        && lhs.gatherer_id == rhs.gatherer_id
        && WithinAbs(lhs.sq_distance, AVAILABLE_ERROR).match(rhs.sq_distance)
        && WithinAbs(lhs.time, AVAILABLE_ERROR).match(rhs.time);
};

struct ContainsEqualGatheringEventsMatcher : Catch::Matchers::MatcherBase<std::vector<GatheringEvent>> {
    ContainsEqualGatheringEventsMatcher(std::vector<GatheringEvent>&& events)
        : events_(std::move(events)) {
            std::sort(events_.begin(), events_.end(), 
                [](const GatheringEvent& lhs, const GatheringEvent& rhs) {
                    return lhs.time < rhs.time;
                });
    }

    bool match(const std::vector<GatheringEvent>& other) const override {        
        return std::equal(events_.begin(), events_.end(), other.begin(), other.end(), are_equal_events);
    }

    std::string describe() const override {
        return "Is equal to expected events: "s + Catch::rangeToString(events_);
    }

private:
    std::vector<GatheringEvent> events_;
};

ContainsEqualGatheringEventsMatcher ContainsEqualGatheringEvents(std::vector<GatheringEvent>&& events) {
    return ContainsEqualGatheringEventsMatcher(std::move(events));
}

// Tests

SCENARIO("Find gather events test") {
    class ItemGathererProviderBase : public ItemGathererProvider {
    public:
        size_t ItemsCount() const override { return items_.size(); }
        Item GetItem(size_t idx) const override { return items_.at(idx); }
        size_t GatherersCount() const override { return gatherers_.size(); }
        Gatherer GetGatherer(size_t idx) const override { return gatherers_.at(idx); }

    public:
        void PushItem(Item&& item) { items_.emplace_back(item); }
        void PushGatherer(Gatherer&& gatherer) { gatherers_.emplace_back(gatherer); }

    private:
        std::vector<Item> items_;
        std::vector<Gatherer> gatherers_;
    };

    GIVEN("a provider") {
        auto provider = std::make_shared<ItemGathererProviderBase>();
        std::vector<GatheringEvent> expected;

        WHEN("there is no objects in provider") {
            CHECK(provider->ItemsCount() == 0);
            CHECK(provider->GatherersCount() == 0);
            
            THEN("there is no collisions") {
                auto result = FindGatherEvents(*provider);
                CHECK(result.empty());
            }

            THEN("throw with wrong idx") {
                CHECK_THROWS(provider->GetItem(1));
                CHECK_THROWS(provider->GetGatherer(1));
            }
        }

        WHEN("item is out the gatherer's way(horizontal)") {
            Item item;
            item.position = geom::Point2D(10.0, 0.0);
            item.width = 2.0;

            Gatherer gatherer;
            gatherer.start_pos = geom::Point2D(0.0, 0.0);
            gatherer.end_pos = geom::Point2D(5.0, 0.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));

            THEN("there is no collisions") {
                auto result = FindGatherEvents(*provider);
                CHECK(result.empty());
            }
        }

        WHEN("item is out the gatherer's way(horizontal)") {
            Item item;
            item.position = geom::Point2D(10.0, 0.0);
            item.width = 2.0;

            Gatherer gatherer;
            gatherer.start_pos = geom::Point2D(0.0, 0.0);
            gatherer.end_pos = geom::Point2D(5.0, 0.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));

            THEN("there is no collisions") {
                auto result = FindGatherEvents(*provider);
                CHECK(result.empty());
            }
        }

        WHEN("item is out the gatherer's way(vertical)") {
            Item item;
            item.position = geom::Point2D(0.0, 10.0);
            item.width = 2.0;

            Gatherer gatherer;
            gatherer.start_pos = geom::Point2D(0.0, 0.0);
            gatherer.end_pos = geom::Point2D(0.0, 5.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));

            THEN("there is no collisions") {
                auto result = FindGatherEvents(*provider);
                CHECK(result.empty());
            }
        }

        WHEN("one gatherer and one item on his way") {
            Item item;
            item.position = geom::Point2D(2.0, 5.0);
            item.width = 1.0;

            Gatherer gatherer;
            gatherer.start_pos = geom::Point2D(0.0, 0.0);
            gatherer.end_pos = geom::Point2D(0.0, 10.0);
            gatherer.width = 2.0;


            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));
            GatheringEvent evt{.item_id = 0,
                                .gatherer_id = 0,
                                .sq_distance = 4,
                                .time = 0.5};
            expected.emplace_back(std::move(evt));
            THEN("there is a collision") {
                // auto collect_res = TryCollectPoint(provider->GetGatherer(0).start_pos, provider->GetGatherer(0).end_pos, provider->GetItem(0).position);
                // std::cout << "sq distance: " << collect_res.sq_distance << ", proj ratio: " << collect_res.proj_ratio << std::endl;
                // if (collect_res.IsCollected(provider->GetGatherer(0).width + provider->GetItem(0).width)) {
                //     std::cout << "collected" << std::endl;
                // }
                // CHECK_THAT(collect_res.proj_ratio, WithinAbs(0.5, 1e-10));
                // CHECK_THAT(collect_res.sq_distance, WithinAbs(4, 1e-10));

                auto result = FindGatherEvents(*provider);
                REQUIRE(!result.empty());
                CHECK_THAT(result, ContainsEqualGatheringEvents(std::move(expected)));
            }
        }

    }
}