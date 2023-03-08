#include "json_helper.h"

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
        std::cout << e.what() << std::endl;
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
        std::cout << e.what() << std::endl;
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
        std::cout << e.what() << std::endl;
    }

    return result;
}

boost::json::object CreateErrorValue(const std::string& code, const std::string& message) {
    boost::json::object val;
    
    val["code"] = code;
    val["message"] = message;

    return val;
}

boost::json::array CreateRoadsArray(const model::Map& map) {
    boost::json::array roads;

    for (const auto& road : map.GetRoads()) {
        boost::json::value roadVal;
        if (road.IsVertical()) {
            roadVal = {
                {"x0", road.GetStart().x},
                {"y0", road.GetStart().y},
                {"y1", road.GetEnd().y}
            };
        } else {
            roadVal = {
                {"x0", road.GetStart().x},
                {"y0", road.GetStart().y},
                {"x1", road.GetEnd().x}
            };
        }
        roads.push_back(std::move(roadVal));
    }

    return roads;
}

boost::json::array CreateBuildingsArray(const model::Map& map) {
    boost::json::array buildings;

    for (const auto& building : map.GetBuildings()) {
        boost::json::value buildingVal {
            {"x", building.GetBounds().position.x},
            {"y", building.GetBounds().position.y},
            {"w", building.GetBounds().size.width},
            {"h", building.GetBounds().size.height}
        };
        buildings.push_back(std::move(buildingVal));
    }

    return buildings;
}

boost::json::array CreateOfficesArray(const model::Map& map) {
    boost::json::array offices;

    for (const auto& office : map.GetOffices()) {
        boost::json::value officeVal{
            {"id", *office.GetId()},
            {"x", office.GetPosition().x},
            {"y", office.GetPosition().y},
            {"offsetX", office.GetOffset().dx},  
            {"offsetY", office.GetOffset().dy}
        };
        offices.push_back(std::move(officeVal));
    }

    return offices;
}

}