#pragma once

#include <filesystem>
#include <boost/json/parse.hpp>

#include "model.h"

namespace json_loader {

bool AddRoadsToMap(model::Map& currentMap, const boost::json::value& mapObj);
bool AddBuildingsToMap(model::Map& currentMap, const boost::json::value& mapObj);
bool AddOfficesToMap(model::Map& currentMap, const boost::json::value& mapObj);
model::Game LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader
