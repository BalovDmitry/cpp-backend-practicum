#include "json_helper.h"
#include "logger.h"
#include "extra_data.h"
#include "model_utils.h"

#include <boost/json/serialize.hpp>
#include <iostream>

namespace json_helper {

bool AddRoadsToMap(model::Map& currentMap, const boost::json::value& mapObj) {
    bool result = false;
    try {
        for (const auto& road : mapObj.at("roads").as_array()) {
            const auto& roadObj = road.as_object();
            
            model::Point start;
            start.x = roadObj.at("x0").as_int64();
            start.y = roadObj.at("y0").as_int64();

            if (roadObj.contains("x1")) {
                model::Coord endCoord = roadObj.at("x1").as_int64();
                currentMap.AddRoad({model::Road::HORIZONTAL, start, endCoord});
            } else {
                model::Coord endCoord = roadObj.at("y1").as_int64();
                currentMap.AddRoad({model::Road::VERTICAL, start, endCoord});
            }
        }

        result = true;
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool AddBuildingsToMap(model::Map& currentMap, const boost::json::value& mapObj) {
    bool result = false;
    try {
        for (const auto& building : mapObj.at("buildings").as_array()) {
            const auto& buildingObj = building.as_object();
            
            model::Rectangle rectangle;
            rectangle.position.x = buildingObj.at("x").as_int64();
            rectangle.position.y = buildingObj.at("y").as_int64();
            rectangle.size.width = buildingObj.at("w").as_int64();
            rectangle.size.height = buildingObj.at("h").as_int64();

            currentMap.AddBuilding(model::Building{std::move(rectangle)});
        }

        result = true;
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool AddOfficesToMap(model::Map& currentMap, const boost::json::value& mapObj) {
    bool result = false;
    try {
        for (const auto& office : mapObj.at("offices").as_array()) {
            const auto& officeObj = office.as_object();
            
            auto id = officeObj.at("id").as_string();
            model::Office::Id currentId({id.data(), id.size()});
            
            model::Point position;
            position.x = officeObj.at("x").as_int64();
            position.y = officeObj.at("y").as_int64();

            model::Offset offset;
            offset.dx = officeObj.at("offsetX").as_int64();
            offset.dy = officeObj.at("offsetY").as_int64();

            currentMap.AddOffice(
                model::Office(std::move(currentId), std::move(position), std::move(offset)));
        }

        result = true;
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool AddExtraData(const model::Map& currentMap, const boost::json::value& mapObj) {
    try {
        const auto loot_array = mapObj.at("lootTypes").as_array();
        return model::ExtraData::GetInstance().AddLootToMap(currentMap.GetId(), loot_array);
    } catch(std::exception& e) {
        logger::LogErrorMessage(e.what());
        return false;
    }

    return false;
}

bool SetMapDogSpeed(model::Map &currentMap, const boost::json::value &mapObj) {
    bool result = false;

    try {
        if (mapObj.as_object().contains("dogSpeed")) {
            auto speed = mapObj.at("dogSpeed").as_double();
            currentMap.SetSpeed(speed);
            result = true;
        }
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool SetDefaultDogSpeed(model::Map& currentMap, const boost::json::value& configObj) {
    if (configObj.as_object().contains("defaultDogSpeed")) {
        currentMap.SetSpeed(configObj.as_object().at("defaultDogSpeed").as_double());
    } else {
        currentMap.SetSpeed(model::DEFAULT_DOG_SPEED);
    }
    return true;
}

bool SetLootGeneratorData(const boost::json::value& configObj) {
    bool result = false;
    
    try {
        auto val = configObj.at("lootGeneratorConfig").as_object();
        auto period = val.at("period").as_double();
        auto probability = val.at("probability").as_double();
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
        if (mapObj.as_object().contains("bagCapacity")) {
            auto bag_capacity = mapObj.at("bagCapacity").as_int64();
            currentMap.SetBagCapacity(bag_capacity);
            result = true;
        }
    } catch (std::exception& e) {
        logger::LogErrorMessage(e.what());
    }

    return result;
}

bool SetDefaultBagCapacity(model::Map &currentMap, const boost::json::value &configObj) {
    if (configObj.as_object().contains("defaultBagCapacity")) {
        currentMap.SetBagCapacity(configObj.as_object().at("defaultBagCapacity").as_int64());
    } else {
        currentMap.SetBagCapacity(model::DEFAULT_BAG_CAPACITY);
    }
    return true;
}

boost::json::array CreateRoadsArray(const model::Map& map) {
    boost::json::array roads;

    for (const auto& road : map.GetRoads()) {
        boost::json::object roadVal;
        
        roadVal["x0"] = road.GetStart().x;
        roadVal["y0"] = road.GetStart().y;
        
        if (road.IsVertical()) {
            roadVal["y1"] = road.GetEnd().y;
        } else {
            roadVal["x1"] = road.GetEnd().x;
        }
        roads.push_back(std::move(roadVal));
    }

    return roads;
}

boost::json::array CreateBuildingsArray(const model::Map& map) {
    boost::json::array buildings;

    for (const auto& building : map.GetBuildings()) {
        boost::json::object buildingVal; 
        
        buildingVal["x"] = building.GetBounds().position.x;
        buildingVal["y"] = building.GetBounds().position.y;
        buildingVal["w"] = building.GetBounds().size.width;
        buildingVal["h"] = building.GetBounds().size.height;

        buildings.push_back(std::move(buildingVal));
    }

    return buildings;
}

boost::json::array CreateOfficesArray(const model::Map& map) {
    boost::json::array offices;

    for (const auto& office : map.GetOffices()) {
        boost::json::object officeVal;

        officeVal["id"] = *office.GetId();
        officeVal["x"] = office.GetPosition().x;
        officeVal["y"] = office.GetPosition().y;
        officeVal["offsetX"] = office.GetOffset().dx;
        officeVal["offsetY"] = office.GetOffset().dy;
        
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
