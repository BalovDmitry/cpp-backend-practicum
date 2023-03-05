#include "request_handler.h"

#include <vector>
#include <string>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split
#include <boost/json/parse.hpp>
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
    std::ostringstream buf;

    bool first = true;
    buf << "[";
    for (const auto& map : game_.GetMaps()) {
        if (!first) 
            bodyText += ", ";
        buf << "{";
        buf << "\"id\": " << "\"" << *map.GetId() << "\", ";
        buf << "\"name\": " << "\"" << map.GetName() << "\"";
        buf << "}";
        first = false;
    }
    buf << "]";
    
    bodyText += buf.str();
    status = http::status::ok;

    return true;
}

bool RequestHandler::MakeGetMapByIdBody(model::Map::Id id, std::string& bodyText, http::status& status) {
    std::ostringstream buf;

    auto map = game_.FindMap(id);
    buf << "{\n";
    if (map) {
        std::vector<std::string> roads;
        for (const auto& road : map->GetRoads()) {
            boost::json::value roadVal;
            if (road.IsVertical()) {
                roadVal = {
                    {"x0", road.GetStart().x},
                    {"y0", road.GetStart().y},
                    {"y0", road.GetEnd().y}
                };
            } else {
                roadVal = {
                    {"x0", road.GetStart().x},
                    {"y0", road.GetStart().y},
                    {"x0", road.GetEnd().x}
                };
            }
            roads.push_back(boost::json::serialize(roadVal));
        }

        std::vector<std::string> buildings;
        for (const auto& building : map->GetBuildings()) {
            boost::json::value buildingVal {
                {"x", building.GetBounds().position.x},
                {"y", building.GetBounds().position.y},
                {"w", building.GetBounds().size.width},
                {"h", building.GetBounds().size.height}
            };
            buildings.push_back(boost::json::serialize(buildingVal));
        }

        std::vector<std::string> offices;
        for (const auto& office : map->GetOffices()) {
            boost::json::value officeVal{
                {"id", *office.GetId()},
                {"x", office.GetPosition().x},
                {"y", office.GetPosition().y},
                {"offsetX", office.GetOffset().dx},  
                {"offsetY", office.GetOffset().dy}
            };
            offices.push_back(boost::json::serialize(officeVal));
        }

        buf << "\t\"id\": " << "\"" << *map->GetId() << "\",";
        buf << "\n";
        buf << "\t\"name\": " << "\"" << map->GetName() << "\",";
        buf << "\n";
        
        buf << "\t" << "\"roads\": [";
        bool firstRoad = true;
        for (const auto& r : roads) {
            if (!firstRoad) 
                buf << ", ";
            buf << "\n";
            buf << "\t\t" << r; 
            firstRoad = false;
        }
        buf << "\n\t],\n";

        buf << "\t" << "\"buildings\": [";
        bool firstB = true;
        for (const auto& b : buildings) {
            if (!firstB) 
                buf << ", "; 
            buf << "\n"; 
            buf << "\t\t" << b;
            firstB = false;
        }
        buf << "\n\t],\n";

        buf << "\t" << "\"offices\": [";
        bool firstOf = true;
        for (const auto& o : offices) {
            if (!firstOf) 
                buf << ", ";
            buf << "\n";    
            buf << "\t\t" << o; 
            firstOf = false;
        }
        buf << "\n\t]\n";
        status = http::status::ok;
    } else {
        buf << "\t\"code\": " << "\"" << "mapNotFound" << "\",\n";
        buf << "\t\"message\": " << "\"" << "Map not found" << "\"\n";
        status = http::status::not_found;
    }

    buf << "}\n";        
    bodyText += buf.str();
    
    return true;
}

bool RequestHandler::MakeBadRequestBody(std::string& bodyText, http::status& status) {
    std::ostringstream buf;
    buf << "{\n";
    buf << "\t\"code\": " << "\"" << "badRequest" << "\",\n";
    buf << "\t\"message\": " << "\"" << "Bad request" << "\"\n";
    buf << "}\n";

    status = http::status::bad_request;
    bodyText += buf.str();
    return true;
}

bool RequestHandler::MakeMethodNotAllowedBody(std::string& bodyText, http::status& status) {
    std::ostringstream buf;
    buf << "{\n";
    buf << "\t\"code\": " << "\"" << "methodNotAllowed" << "\",\n";
    buf << "\t\"message\": " << "\"" << "Method not allowed" << "\"\n";
    buf << "}\n";

    status = http::status::method_not_allowed;
    bodyText += buf.str();

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
    if ((splittedRequest.size() == 3 || splittedRequest.size() == 4)
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
        && splittedRequest.size() == 3) {
        
        result = RequestType::GET_MAP_LIST;
    }
    
    else if (CheckRequestCorrectness(splittedRequest)
            && splittedRequest.size() == 4
            && splittedRequest[3].starts_with("map")
            && splittedRequest[3].size() == 4) {
        
        result = RequestType::GET_MAP_BY_ID;
    }
    
    return result;
}

}  // namespace http_handler
