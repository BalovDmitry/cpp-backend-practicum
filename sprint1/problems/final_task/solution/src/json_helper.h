#pragma once

#include <boost/json/parse.hpp>
#include <string>

#include "model.h"

namespace json_helper {
    bool AddRoadsToMap(model::Map& currentMap, const boost::json::value& mapObj);
    bool AddBuildingsToMap(model::Map& currentMap, const boost::json::value& mapObj);
    bool AddOfficesToMap(model::Map& currentMap, const boost::json::value& mapObj);

    boost::json::array CreateRoadsArray(const model::Map& map);
    boost::json::array CreateBuildingsArray(const model::Map& map);
    boost::json::array CreateOfficesArray(const model::Map& map);

    template<typename CodeType, typename MessageType> 
    boost::json::object CreateErrorValue(CodeType&& code, MessageType&& message) {
        boost::json::object val;
    
        val["code"] = std::forward<CodeType>(code);
        val["message"] = std::forward<MessageType>(message);

        return val;
    }
}
