#pragma once

#include <boost/json/parse.hpp>
#include <string>

#include "model.h"

namespace json_helper {
    bool AddRoadsToMap(model::Map& currentMap, const boost::json::value& mapObj);
    bool AddBuildingsToMap(model::Map& currentMap, const boost::json::value& mapObj);
    bool AddOfficesToMap(model::Map& currentMap, const boost::json::value& mapObj);

    boost::json::object CreateErrorValue(const std::string& code, const std::string& message);
    boost::json::array CreateRoadsArray(const model::Map& map);
    boost::json::array CreateBuildingsArray(const model::Map& map);
    boost::json::array CreateOfficesArray(const model::Map& map);

    std::string CreateRoadsArrayString(const model::Map& map);

}