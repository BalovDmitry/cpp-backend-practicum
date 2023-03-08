#include "request_handler.h"
#include "json_helper.h"

#include <vector>
#include <string>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split
#include <boost/json/serialize.hpp>

#include <sstream>
#include <iostream>

namespace http_handler {

StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
    const auto text_response = [this, &req](http::status status, std::string_view text) {
        return this->MakeStringResponse(status, text, req.version(), req.keep_alive());
    };

    http::status status;
    std::string bodyText;
    
    if (req.method() == http::verb::get) {
        if (!req.target().empty()) {
            auto uriWords = GetVectorFromTarget(req.target());
            MakeBodyText(uriWords, GetRequestType(uriWords), bodyText, status);
        }
    } else {
        MakeMethodNotAllowedBody(bodyText, status);
    }

    return text_response(status, bodyText);
}

void RequestHandler::MakeBodyText(const std::vector<std::string>& splittedRequest, RequestType requestType, std::string& bodyText, http::status& status) {
    switch (requestType)
    {
        case RequestType::GET_MAP_LIST: {
            MakeGetMapListBody(bodyText, status);
            break;
        }

        case RequestType::GET_MAP_BY_ID: {
            model::Map::Id id{ splittedRequest.back() };
            MakeGetMapByIdBody(id, bodyText, status);
            break;
        }

        case RequestType::UNKNOWN: {
            MakeBadRequestBody(bodyText, status);
            break;
        }
    }
}

bool RequestHandler::MakeGetMapListBody(std::string& bodyText, http::status& status) {
    boost::json::array jsonList; 
    for (const auto& map : game_.GetMaps()) {
        boost::json::object val;
        val["id"] = *map.GetId();
        val["name"] = map.GetName();
        jsonList.push_back(std::move(val));
    }
    
    bodyText += boost::json::serialize(jsonList);
    status = http::status::ok;

    return true;
}

bool RequestHandler::MakeGetMapByIdBody(model::Map::Id id, std::string& bodyText, http::status& status) {
    boost::json::object val;
    
    auto map = game_.FindMap(id);
    if (map) {
        auto roads = json_helper::CreateRoadsArray(*map);
        auto buildings = json_helper::CreateBuildingsArray(*map);
        auto offices = json_helper::CreateOfficesArray(*map);

        val["id"] = *map->GetId();
        val["name"] = map->GetName();
        val["roads"] = boost::json::serialize(roads);
        val["buildings"] = boost::json::serialize(buildings);
        val["offices"] = boost::json::serialize(offices);

        status = http::status::ok;
    } else {
        val = json_helper::CreateErrorValue("mapNotFound", "Map not found");
        status = http::status::not_found;
    }

    bodyText += boost::json::serialize(val);
    
    return true;
}


bool RequestHandler::MakeBadRequestBody(std::string& bodyText, http::status& status) {
    bodyText += boost::json::serialize(json_helper::CreateErrorValue("badRequest", "Bad request"));
    status = http::status::bad_request;

    return true;
}

bool RequestHandler::MakeMethodNotAllowedBody(std::string& bodyText, http::status& status) {
    bodyText += boost::json::serialize(json_helper::CreateErrorValue("methodNotAllowed", "Method not allowed"));
    status = http::status::method_not_allowed;

    return true;
}

StringResponse RequestHandler::MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                bool keep_alive,
                                std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

std::vector<std::string> RequestHandler::GetVectorFromTarget(std::string_view target) {
    std::vector<std::string> result;
    boost::split(result, std::string_view(target.data() + 1, target.size() - 1), boost::is_any_of("/"), boost::token_compress_on);
    return result;
}

bool RequestHandler::CheckRequestCorrectness(const std::vector<std::string>& splittedRequest) {
    if ((splittedRequest.size() == RequestTypeSize::GET_MAP_BY_ID || splittedRequest.size() == RequestTypeSize::GET_MAP_LIST)
        && splittedRequest[0] == "api"
        && splittedRequest[1] == "v1"
        && splittedRequest[2] == "maps") {
        
        return true;
    }
    return false;
}

RequestType RequestHandler::GetRequestType(const std::vector<std::string>& splittedRequest) {
    RequestType result = RequestType::UNKNOWN;

    if (CheckRequestCorrectness(splittedRequest)
        && splittedRequest.size() == RequestTypeSize::GET_MAP_LIST) {
        
        result = RequestType::GET_MAP_LIST;
    }
    
    else if (CheckRequestCorrectness(splittedRequest)
            && splittedRequest.size() == RequestTypeSize::GET_MAP_BY_ID
            && splittedRequest.back().starts_with("map")) {
        
        result = RequestType::GET_MAP_BY_ID;
    }
    
    return result;
}

}  // namespace http_handler
