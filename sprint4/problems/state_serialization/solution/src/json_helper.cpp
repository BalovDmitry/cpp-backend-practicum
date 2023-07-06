#include "json_helper.h"
#include "logger.h"
#include "extra_data.h"
#include "model_utils.h"

#include <boost/json/serialize.hpp>
#include <iostream>

namespace json_helper {

// Protocol constants
static const std::string ROADS_STRING = "roads";
static const std::string BUILDINGS_STRING = "buildings";
static const std::string OFFICES_STRING = "offices";
static const std::string LOOT_TYPES_STRING = "lootTypes";
static const std::string DOG_SPEED_STRING = "dogSpeed";
static const std::string DEFAULT_DOG_SPEED_STRING = "defaultDogSpeed";
static const std::string LOOT_GENERATOR_STRING = "lootGeneratorConfig";
static const std::string BAG_CAPACITY_STRING = "bagCapacity";
static const std::string DEFAULT_BAG_CAPACITY_STRING = "defaultBagCapacity";

static const std::string POSITION_X = "x";
static const std::string POSITION_Y = "y";
static const std::string START_X = "x0";
static const std::string START_Y = "y0";
static const std::string END_X = "x1";
static const std::string END_Y = "y1";
static const std::string WIDTH = "w";
static const std::string HEIGTH = "h";
static const std::string ID = "id";
static const std::string OFFSET_X = "offsetX";
static const std::string OFFSET_Y = "offsetY";
static const std::string PERIOD = "period";
static const std::string PROBABILITY = "probability";

bool AddRoadsToMap(model::Map& currentMap, const boost::json::value& mapObj) {
    bool result = false;
    try {
        for (const auto& road : mapObj.at(ROADS_STRING).as_array()) {
            const auto& roadObj = road.as_object();
            
            model::Point start;
            start.x = roadObj.at(START_X).as_int64();
            start.y = roadObj.at(START_Y).as_int64();

            if (roadObj.contains(END_X)) {
                model::Coord endCoord = roadObj.at(END_X).as_int64();
                currentMap.AddRoad({model::Road::HORIZONTAL, start, endCoord});
            } else {
                model::Coord endCoord = roadObj.at(END_Y).as_int64();
                currentMap.AddRoad({model::Road::VERTICAL, start, endCoord});
            }
        }

        result = true;
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
        throw std::runtime_error("Roads havent'been added!");
    }

    return result;
}

bool AddBuildingsToMap(model::Map& currentMap, const boost::json::value& mapObj) {
    bool result = false;
    try {
        for (const auto& building : mapObj.at(BUILDINGS_STRING).as_array()) {
            const auto& buildingObj = building.as_object();
            
            model::Rectangle rectangle;
            rectangle.position.x = buildingObj.at(POSITION_X).as_int64();
            rectangle.position.y = buildingObj.at(POSITION_Y).as_int64();
            rectangle.size.width = buildingObj.at(WIDTH).as_int64();
            rectangle.size.height = buildingObj.at(HEIGTH).as_int64();

            currentMap.AddBuilding(model::Building{std::move(rectangle)});
        }

        result = true;
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
        throw std::runtime_error("Buildings havent'been added!");
    }

    return result;
}

bool AddOfficesToMap(model::Map& currentMap, const boost::json::value& mapObj) {
    bool result = false;
    try {
        for (const auto& office : mapObj.at(OFFICES_STRING).as_array()) {
            const auto& officeObj = office.as_object();
            
            auto id = officeObj.at(ID).as_string();
            model::Office::Id currentId({id.data(), id.size()});
            
            model::Point position;
            position.x = officeObj.at(POSITION_X).as_int64();
            position.y = officeObj.at(POSITION_Y).as_int64();

            model::Offset offset;
            offset.dx = officeObj.at(OFFSET_X).as_int64();
            offset.dy = officeObj.at(OFFSET_Y).as_int64();

            currentMap.AddOffice(
                model::Office(std::move(currentId), std::move(position), std::move(offset)));
        }

        result = true;
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
        throw std::runtime_error("Offices havent'been added!");
    }

    return result;
}

bool AddExtraData(const model::Map& currentMap, const boost::json::value& mapObj) {
    try {
        const auto loot_array = mapObj.at(LOOT_TYPES_STRING).as_array();
        return model::ExtraData::GetInstance().AddLootToMap(currentMap.GetId(), loot_array);
    } catch(std::exception& e) {
        logger::LogErrorMessage(e.what());
        throw std::runtime_error("Extra data hasn't been added!");
    }

    return false;
}

bool SetMapDogSpeed(model::Map &currentMap, const boost::json::value &mapObj) {
    bool result = false;

    try {
        if (mapObj.as_object().contains(DOG_SPEED_STRING)) {
            auto speed = mapObj.at(DOG_SPEED_STRING).as_double();
            currentMap.SetSpeed(speed);
            result = true;
        }
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool SetDefaultDogSpeed(model::Map& currentMap, const boost::json::value& configObj) {
    if (configObj.as_object().contains(DEFAULT_DOG_SPEED_STRING)) {
        currentMap.SetSpeed(configObj.as_object().at(DEFAULT_DOG_SPEED_STRING).as_double());
    } else {
        currentMap.SetSpeed(model::DEFAULT_DOG_SPEED);
    }
    return true;
}

bool SetLootGeneratorData(const boost::json::value& configObj) {
    bool result = false;
    
    try {
        auto val = configObj.at(LOOT_GENERATOR_STRING).as_object();
        auto period = val.at(PERIOD).as_double();
        auto probability = val.at(PROBABILITY).as_double();
        model::ExtraData::GetInstance().SetLootGeneratorData(period, probability);
        result = true;
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool SetMapBagCapacity(model::Map &currentMap, const boost::json::value &mapObj) {
    bool result = false;

    try {
        if (mapObj.as_object().contains(BAG_CAPACITY_STRING)) {
            auto bag_capacity = mapObj.at(BAG_CAPACITY_STRING).as_int64();
            currentMap.SetBagCapacity(bag_capacity);
            result = true;
        }
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool SetDefaultBagCapacity(model::Map &currentMap, const boost::json::value &configObj) {
    if (configObj.as_object().contains(DEFAULT_BAG_CAPACITY_STRING)) {
        currentMap.SetBagCapacity(configObj.as_object().at(DEFAULT_BAG_CAPACITY_STRING).as_int64());
    } else {
        currentMap.SetBagCapacity(model::DEFAULT_BAG_CAPACITY);
    }
    return true;
}

boost::json::array CreateRoadsArray(const model::Map& map) {
    boost::json::array roads;

    for (const auto& road : map.GetRoads()) {
        boost::json::object roadVal;
        
        roadVal[START_X] = road.GetStart().x;
        roadVal[START_Y] = road.GetStart().y;
        
        if (road.IsVertical()) {
            roadVal[END_Y] = road.GetEnd().y;
        } else {
            roadVal[END_X] = road.GetEnd().x;
        }
        roads.push_back(std::move(roadVal));
    }

    return roads;
}

boost::json::array CreateBuildingsArray(const model::Map& map) {
    boost::json::array buildings;

    for (const auto& building : map.GetBuildings()) {
        boost::json::object buildingVal; 
        
        buildingVal[POSITION_X] = building.GetBounds().position.x;
        buildingVal[POSITION_Y] = building.GetBounds().position.y;
        buildingVal[WIDTH] = building.GetBounds().size.width;
        buildingVal[HEIGTH] = building.GetBounds().size.height;

        buildings.push_back(std::move(buildingVal));
    }

    return buildings;
}

boost::json::array CreateOfficesArray(const model::Map& map) {
    boost::json::array offices;

    for (const auto& office : map.GetOffices()) {
        boost::json::object officeVal;

        officeVal[ID] = *office.GetId();
        officeVal[POSITION_X] = office.GetPosition().x;
        officeVal[POSITION_Y] = office.GetPosition().y;
        officeVal[OFFSET_X] = office.GetOffset().dx;
        officeVal[OFFSET_Y] = office.GetOffset().dy;
        
        offices.push_back(std::move(officeVal));
    }

    return offices;
}

boost::json::object CreateStartServerValue(net::ip::port_type port, const net::ip::address& address) {
    boost::json::object val;

    val["port"] = port;
    val["address"] = address.to_string();

    return val;
}

boost::json::object CreateStopServerValue(uint8_t code, std::optional<std::string> exception) {
    boost::json::object val;

    val["code"] = code;
    if (exception.has_value()) {
        val["exception"] = exception.value();
    }

    return val;
}

boost::json::object CreateNetworkErrorValue(int code, const std::string&  text, const std::string&  where) {
    boost::json::object val;

    val["code"] = code;
    val["text"] = text;
    val["where"] = where;

    return val;
}

boost::json::object CreateRequestValue(const tcp::endpoint& endpoint, const std::string& uri, const std::string&  method) {
    boost::json::object val;

    val["ip"] = endpoint.address().to_string();
    val["URI"] = uri;
    val["method"] = method;

    return val;
}

boost::json::object CreateResponseValue(int response_time, int code, const std::string&  content_type) {
    boost::json::object val;

    val["response time"] = response_time;
    val["code"] = code;
    val["content_type"] = content_type;

    return val;
}

boost::json::object CreateLostObjectValue(unsigned type, const model::Position& position) {
    boost::json::object val;

    val["type"] = type;
    val["pos"] = CreateCoordArray(position.x, position.y);

    return val;
}

boost::json::array CreateCoordArray(double x, double y) {
    boost::json::array res;
    res.push_back(x);
    res.push_back(y);
    return res;
}

boost::json::array CreateBagArray(const std::unordered_map<unsigned, model::LootItem>& bag) {
    boost::json::array res;
    for (const auto& [id, item] : bag) {
        boost::json::object temp;
        temp["id"] = id;
        temp["type"] = item.type;
        res.emplace_back(std::move(temp));
    }
    return res;
}

}
