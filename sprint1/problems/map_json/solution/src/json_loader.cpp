#include "json_loader.h"

#include <boost/json/parse.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace json_loader {

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

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game;

    std::ifstream fStream(json_path);
    std::stringstream buf;
    buf << fStream.rdbuf();

    boost::system::error_code ec;
    auto val = boost::json::parse(buf.str(), ec);
    
    try {
        auto mapsObj = val.as_object()["maps"];
        for (const auto& map : mapsObj.as_array()) {
            const auto& mapObj = map.as_object();
            
            auto id = mapObj.at("id").as_string();
            auto name = mapObj.at("name").as_string();

            model::Map::Id currentId({id.data(), id.size()});
            model::Map currentMap(currentId, { name.data(), name.size() });

            if (!AddRoadsToMap(currentMap, mapObj)) {
                throw std::runtime_error("Roads havent'been added!");
            }

            if (!AddBuildingsToMap(currentMap, mapObj)) {
                throw std::runtime_error("Buildings havent'been added!");
            }

            if (!AddOfficesToMap(currentMap, mapObj)) {
                throw std::runtime_error("Offices havent'been added!");
            }

            game.AddMap(std::move(currentMap));
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return game;
}

}  // namespace json_loader
