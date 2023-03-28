#include "json_loader.h"
#include "json_helper.h"

#include <boost/json/parse.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game;

    try {
        std::ifstream fStream(json_path);
        if (!fStream) {
            throw std::runtime_error("Couldn't open json file!");
        }

        std::stringstream buf;
        buf << fStream.rdbuf();

        boost::system::error_code ec;
        auto val = boost::json::parse(buf.str(), ec);

        if (ec) {
            throw std::runtime_error("Couldn't parse json file!");
        }

        auto mapsObj = val.as_object()["maps"];
        for (const auto& map : mapsObj.as_array()) {
            const auto& mapObj = map.as_object();
            
            auto id = mapObj.at("id").as_string();
            auto name = mapObj.at("name").as_string();

            model::Map::Id currentId({id.data(), id.size()});
            model::Map currentMap(currentId, { name.data(), name.size() });

            if (!json_helper::AddRoadsToMap(currentMap, mapObj)) {
                throw std::runtime_error("Roads havent'been added!");
            }

            if (!json_helper::AddBuildingsToMap(currentMap, mapObj)) {
                throw std::runtime_error("Buildings havent'been added!");
            }

            if (!json_helper::AddOfficesToMap(currentMap, mapObj)) {
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
