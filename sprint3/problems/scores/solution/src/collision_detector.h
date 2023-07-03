#pragma once

#include "geom.h"
#include "model_utils.h"

#include <algorithm>
#include <vector>

namespace collision_detector {

struct CollectionResult {
    CollectionResult(double sq_distance, double proj_ratio) {
        this->sq_distance = sq_distance;
        this->proj_ratio = proj_ratio;
    }

    bool IsCollected(double collect_radius) const {
        return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= collect_radius * collect_radius;
    }

    // квадрат расстояния до точки
    double sq_distance;

    // доля пройденного отрезка
    double proj_ratio;
};

// Движемся из точки a в точку b и пытаемся подобрать точку c.
// Эта функция реализована в уроке.
CollectionResult TryCollectPoint(model::Position a, model::Position b, model::Position c);

struct Item {
    Item() = default;
    Item(const model::Position& position, double width, unsigned id, bool is_base = false) {
        this->position = position;
        this->width = width;
        this->id = id;
        this->is_base = is_base;
    };

    model::Position position;
    double width;
    unsigned id;
    bool is_base;
};

struct Gatherer {
    Gatherer() = default;
    Gatherer(const model::Position& start_pos, const model::Position& end_pos, double width, unsigned id) {
        this->start_pos = start_pos;
        this->end_pos = end_pos;
        this->width = width;
        this->id = id;
    };

    model::Position start_pos;
    model::Position end_pos;
    double width;
    unsigned id;
};

class ItemGathererProvider {
protected:
    ~ItemGathererProvider() = default;

public:
    virtual size_t ItemsCount() const = 0;
    virtual Item GetItem(size_t idx) const = 0;
    virtual size_t GatherersCount() const = 0;
    virtual Gatherer GetGatherer(size_t idx) const = 0;
};

struct GatheringEvent {
    size_t item_id;
    size_t gatherer_id;
    double sq_distance;
    double time;
    bool is_collision_with_base;
};

// Base implementation of provider
class ItemGathererProviderBase : public ItemGathererProvider {
public:
    size_t ItemsCount() const override { return items_.size(); }
    Item GetItem(size_t idx) const override { return items_.at(idx); }
    size_t GatherersCount() const override { return gatherers_.size(); }
    Gatherer GetGatherer(size_t idx) const override { return gatherers_.at(idx); }

public:
    void PushItem(Item&& item) { items_.emplace_back(item); }
    void PushGatherer(Gatherer&& gatherer) { gatherers_.emplace_back(gatherer); }
    void Clear() { items_.clear(); gatherers_.clear(); }

private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};

// Эту функцию вам нужно будет реализовать в соответствующем задании.
// При проверке ваших тестов она не нужна - функция будет линковаться снаружи.
std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider);

}  // namespace collision_detector