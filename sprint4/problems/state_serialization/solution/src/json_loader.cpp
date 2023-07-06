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
        auto configObj = boost::json::parse(buf.str(), ec);

        if (ec) {
            throw std::runtime_error("Couldn't parse json file!");
        }

        if (!json_helper::SetLootGeneratorData(configObj)) {
            throw std::runtime_error("Loot generator data isn't set!");
        }

        auto mapsObj = configObj.as_object()["maps"];
        for (const auto& map : mapsObj.as_array()) {
            const auto& mapObj = map.as_object();
            
            auto id = mapObj.at("id").as_string();
            auto name = mapObj.at("name").as_string();

            model::Map::Id currentId({id.data(), id.size()});
            model::Map currentMap(currentId, { name.data(), name.size() });

            json_helper::AddRoadsToMap(currentMap, mapObj);
            json_helper::AddBuildingsToMap(currentMap, mapObj);
            json_helper::AddOfficesToMap(currentMap, mapObj);
            json_helper::AddExtraData(currentMap, mapObj);

            if (!json_helper::SetMapDogSpeed(currentMap, mapObj)) {
                json_helper::SetDefaultDogSpeed(currentMap, configObj);
            }

            if (!json_helper::SetMapBagCapacity(currentMap, mapObj)) {
                json_helper::SetDefaultBagCapacity(currentMap, configObj);
            }

            game.AddMap(std::move(currentMap));
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return game;
}

}  // namespace json_loader
