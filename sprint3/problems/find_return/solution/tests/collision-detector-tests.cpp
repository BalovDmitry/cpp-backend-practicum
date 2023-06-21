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
            item.position = model::Position(10.0, 0.0);
            item.width = 2.0;

            Gatherer gatherer;
            gatherer.start_pos = model::Position(0.0, 0.0);
            gatherer.end_pos = model::Position(5.0, 0.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));

            THEN("there is no collisions") {
                CHECK(FindGatherEvents(*provider).empty());
            }
        }

        WHEN("item is out the gatherer's way(horizontal)") {
            Item item;
            item.position = model::Position(10.0, 0.0);
            item.width = 2.0;

            Gatherer gatherer;
            gatherer.start_pos = model::Position(0.0, 0.0);
            gatherer.end_pos = model::Position(5.0, 0.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));

            THEN("there is no collisions") {
                CHECK(FindGatherEvents(*provider).empty());
            }
        }

        WHEN("item is out the gatherer's way(vertical)") {
            Item item;
            item.position = model::Position(0.0, 10.0);
            item.width = 2.0;

            Gatherer gatherer;
            gatherer.start_pos = model::Position(0.0, 0.0);
            gatherer.end_pos = model::Position(0.0, 5.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));

            THEN("there is no collisions") {
                CHECK(FindGatherEvents(*provider).empty());
            }
        }

        WHEN("one gatherer and one item on his way") {
            Item item;
            item.position = model::Position(2.0, 5.0);
            item.width = 1.0;

            Gatherer gatherer;
            gatherer.start_pos = model::Position(0.0, 0.0);
            gatherer.end_pos = model::Position(0.0, 10.0);
            gatherer.width = 2.0;


            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer));
            GatheringEvent evt{.item_id = 0,
                                .gatherer_id = 0,
                                .sq_distance = 4,
                                .time = 0.5};
            expected.emplace_back(std::move(evt));
            THEN("there is a collision") {
                CHECK_THAT(FindGatherEvents(*provider), ContainsEqualGatheringEvents(std::move(expected)));
            }
        }

        WHEN("one gatherer and one item is on his way and one is out his way") {
            Item item1;
            item1.position = model::Position(10.0, 5.0);
            item1.width = 1.0;
            item1.id = 0;

            Item item2;
            item2.position = model::Position(2.0, 5.0);
            item2.width = 1.0;
            item2.id = 1;

            Gatherer gatherer;
            gatherer.start_pos = model::Position(0.0, 0.0);
            gatherer.end_pos = model::Position(0.0, 10.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item1));
            provider->PushItem(std::move(item2));
            provider->PushGatherer(std::move(gatherer));
            
            GatheringEvent evt{.item_id = 1,
                                .gatherer_id = 0,
                                .sq_distance = 4,
                                .time = 0.5};
            expected.emplace_back(std::move(evt));
            
            THEN("there is only one collision") {
                CHECK_THAT(FindGatherEvents(*provider), ContainsEqualGatheringEvents(std::move(expected)));
            }
        }

        WHEN("one gatherer and three items are on his way") {
            Item item1;
            item1.position = model::Position(1.0, 1.0);
            item1.width = 1.0;
            item1.id = 0;

            Item item2;
            item2.position = model::Position(1.5, 5.0);
            item2.width = 1.0;
            item2.id = 1;

            Item item3;
            item3.position = model::Position(1.5, 10.0);
            item3.width = 1.0;
            item3.id = 2;

            Gatherer gatherer;
            gatherer.start_pos = model::Position(0.0, 0.0);
            gatherer.end_pos = model::Position(0.0, 10.0);
            gatherer.width = 2.0;

            provider->PushItem(std::move(item1));
            provider->PushItem(std::move(item2));
            provider->PushItem(std::move(item3));
            provider->PushGatherer(std::move(gatherer));
            
            GatheringEvent evt1{.item_id = 0,
                                .gatherer_id = 0,
                                .sq_distance = 1,
                                .time = 0.1};
            
            GatheringEvent evt2{.item_id = 1,
                                .gatherer_id = 0,
                                .sq_distance = 2.25,
                                .time = 0.5};

            GatheringEvent evt3{.item_id = 2,
                                .gatherer_id = 0,
                                .sq_distance = 2.25,
                                .time = 1};
            expected.emplace_back(std::move(evt1));
            expected.emplace_back(std::move(evt2));
            expected.emplace_back(std::move(evt3));
            
            THEN("there are 3 collisions") {
                CHECK_THAT(FindGatherEvents(*provider), ContainsEqualGatheringEvents(std::move(expected)));
            }
        }

        WHEN("two gatherers and one item is out them way") {
            Item item;
            item.position = model::Position(4.0, 5.0);
            item.width = 1.0;
            item.id = 0;

            Gatherer gatherer1;
            gatherer1.start_pos = model::Position(0.0, 0.0);
            gatherer1.end_pos = model::Position(0.0, 10.0);
            gatherer1.width = 2.0;      

            Gatherer gatherer2;
            gatherer2.start_pos = model::Position(0.0, 2.0);
            gatherer2.end_pos = model::Position(0.0, 6.0);
            gatherer2.width = 2.0;

            THEN("there is no collisions") {
                CHECK_THAT(FindGatherEvents(*provider), ContainsEqualGatheringEvents(std::move(expected)));
            }
        }

        WHEN("two gatherers and one item is on them way") {
            Item item;
            item.position = model::Position(2.0, 5.0);
            item.width = 1.0;
            item.id = 0;

            Gatherer gatherer1;
            gatherer1.start_pos = model::Position(0.0, 0.0);
            gatherer1.end_pos = model::Position(0.0, 10.0);
            gatherer1.width = 2.0;      

            Gatherer gatherer2;
            gatherer2.start_pos = model::Position(0.0, 2.0);
            gatherer2.end_pos = model::Position(0.0, 6.0);
            gatherer2.width = 2.0;

            provider->PushItem(std::move(item));
            provider->PushGatherer(std::move(gatherer1));
            provider->PushGatherer(std::move(gatherer2));

            GatheringEvent evt1{.item_id = 0,
                                .gatherer_id = 0,
                                .sq_distance = 4,
                                .time = 0.5};
            
            GatheringEvent evt2{.item_id = 0,
                                .gatherer_id = 1,
                                .sq_distance = 4,
                                .time = 0.75};

            expected.emplace_back(std::move(evt1));
            expected.emplace_back(std::move(evt2));

            THEN("there are two collisions") {
                CHECK_THAT(FindGatherEvents(*provider), ContainsEqualGatheringEvents(std::move(expected)));
            }
        }

        WHEN("two gatherers and two items on them way") {
            Item item1;
            item1.position = model::Position(2.0, 5.0);
            item1.width = 1.0;
            item1.id = 0;

            Item item2;
            item2.position = model::Position(1.0, 6.0);
            item2.width = 1.0;
            item2.id = 1;

            Gatherer gatherer1;
            gatherer1.start_pos = model::Position(0.0, 0.0);
            gatherer1.end_pos = model::Position(0.0, 10.0);
            gatherer1.width = 2.0;      

            Gatherer gatherer2;
            gatherer2.start_pos = model::Position(0.0, 2.0);
            gatherer2.end_pos = model::Position(0.0, 6.0);
            gatherer2.width = 2.0;

            provider->PushItem(std::move(item1));
            provider->PushItem(std::move(item2));
            provider->PushGatherer(std::move(gatherer1));
            provider->PushGatherer(std::move(gatherer2));

            GatheringEvent evt1{.item_id = 0,
                                .gatherer_id = 0,
                                .sq_distance = 4,
                                .time = 0.5};
            
            GatheringEvent evt2{.item_id = 1,
                                .gatherer_id = 0,
                                .sq_distance = 1,
                                .time = 0.6};

            GatheringEvent evt3{.item_id = 0,
                                .gatherer_id = 1,
                                .sq_distance = 4,
                                .time = 0.75};
            
            GatheringEvent evt4{.item_id = 1,
                                .gatherer_id = 1,
                                .sq_distance = 1,
                                .time = 1};

            expected.emplace_back(std::move(evt1));
            expected.emplace_back(std::move(evt2));
            expected.emplace_back(std::move(evt3));
            expected.emplace_back(std::move(evt4));

            THEN("there are four collisions") {
                CHECK_THAT(FindGatherEvents(*provider), ContainsEqualGatheringEvents(std::move(expected)));
            }
            
        }

    }
}