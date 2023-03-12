#include "request_handler.h"
#include "json_helper.h"

#include <vector>
#include <string>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split
#include <boost/json/serialize.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

namespace http_handler {

StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
    const auto text_response = [this, &req](http::status status, std::string_view text, std::string_view contentType) {
        return this->MakeStringResponse(status, text, req.version(), req.keep_alive());
    };

    http::status status;
    std::string bodyText;
    std::string_view contentType;

    auto splittedRequest = GetVectorFromTarget(req.target());

    if (req.method() == http::verb::get) {
        if (IsApiRequest(splittedRequest)) {
            MakeBodyTextApi(splittedRequest, GetRequestTypeApi(splittedRequest), bodyText, status);
            contentType = ContentType::APP_JSON;
        } else {
            SetResponseDataStaticFile(splittedRequest, GetRequestTypeApi(splittedRequest), bodyText, contentType, status);

            // MakeBodyTextStaticFile(splittedRequest, GetRequestTypeApi(splittedRequest), bodyText, status);
            // contentType = GetContentType(splittedRequest);
            // std::cout << "Splitted request: " << std::endl;
            // for (const auto& w : splittedRequest) {
            //     std::cout << w << std::endl;
            // }
            // std::cout << "Body: " << bodyText << std::endl;
            // std::cout << "Content-Type: " << contentType << std::endl;
        }
    
    } else if(req.method() == http::verb::head) {
        //!TODO: create body for HEAD
    } 
    else {
        MakeMethodNotAllowedBody(bodyText, status);
    }

    return text_response(status, bodyText, contentType);
}


void RequestHandler::SetResponseDataStaticFile(
    const std::vector<std::string>& splittedRequest, 
    RequestTypeApi requestType, 
    std::string& bodyText,
    std::string_view& contentType,
    http::status& status) {

    auto relPath = path_helper::GetRelPathFromRequest(splittedRequest);
    auto absPath = path_helper::GetAbsPath(basePath_, relPath);

    std::ifstream fStream(absPath);
    if (fStream) {
        if (!path_helper::IsSubPath(absPath, basePath_)) {
            MakeBadRequestBody(bodyText, status);
            contentType = ContentType::TEXT_PLAIN;
            return;
        }
        //std::cout << "File " << absPath << " was successfully open!" << std::endl;

        std::stringstream buffer;
        buffer << fStream.rdbuf();
        bodyText += buffer.str();
        contentType = GetContentType(splittedRequest);
        status = http::status::ok;
    } else {
        //std::cout << "Failed to open file "sv << absPath << std::endl;
        MakeFileNotFoundBody(bodyText, status);
        contentType = ContentType::TEXT_PLAIN;
    }
}


void RequestHandler::MakeBodyTextApi(
    const std::vector<std::string>& splittedRequest, 
    RequestTypeApi requestType, 
    std::string& bodyText, 
    http::status& status) {
    
    switch (requestType)
    {
        case RequestTypeApi::GET_MAP_LIST: {
            MakeGetMapListBody(bodyText, status);
            break;
        }

        case RequestTypeApi::GET_MAP_BY_ID: {
            model::Map::Id id{ splittedRequest.back() };
            MakeGetMapByIdBody(id, bodyText, status);
            break;
        }
        
        case RequestTypeApi::UNKNOWN: {
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
        val["id"] = *map->GetId();
        val["name"] = map->GetName();
        val["roads"] = json_helper::CreateRoadsArray(*map);
        val["buildings"] = json_helper::CreateBuildingsArray(*map);
        val["offices"] = json_helper::CreateOfficesArray(*map);

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

bool RequestHandler::MakeFileNotFoundBody(std::string& bodyText, http::status& status) {
    bodyText += boost::json::serialize(json_helper::CreateErrorValue("fileNotFound", "File not  found"));
    status = http::status::not_found;

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

    if (target.size() > 1) {
        boost::split(result, std::string_view(target.data() + 1, target.size() - 1), boost::is_any_of("/\0"), boost::token_compress_on);
    }

    return result;
}

bool RequestHandler::CheckRequestCorrectnessApi(const std::vector<std::string>& splittedRequest) {
    if ((splittedRequest.size() == RequestTypeApiSize::GET_MAP_BY_ID || splittedRequest.size() == RequestTypeApiSize::GET_MAP_LIST)
        && splittedRequest[0] == "api"
        && splittedRequest[1] == "v1"
        && splittedRequest[2] == "maps") {
        
        return true;
    }
    return false;
}

RequestTypeApi RequestHandler::GetRequestTypeApi(const std::vector<std::string>& splittedRequest) {
    RequestTypeApi result = RequestTypeApi::UNKNOWN;

    if (CheckRequestCorrectnessApi(splittedRequest)
        && splittedRequest.size() == RequestTypeApiSize::GET_MAP_LIST) {
        
        result = RequestTypeApi::GET_MAP_LIST;
    }
    
    else if (CheckRequestCorrectnessApi(splittedRequest)
            && splittedRequest.size() == RequestTypeApiSize::GET_MAP_BY_ID
            && splittedRequest.back().starts_with("map")) {
        
        result = RequestTypeApi::GET_MAP_BY_ID;
    }
    
    return result;
}

std::string_view RequestHandler::GetContentType(const std::vector<std::string>& splittedRequest) {
    std::string_view result = ContentType::UNKNOWN;

    if (splittedRequest.empty()) {
        return ContentType::TEXT_HTML;
    }

    auto ext = DetectFileExtension(splittedRequest.back());
    if (ExtensionToContentType.contains(ext)) {
        result = ExtensionToContentType.at(ext);
    } else {
        result = ContentType::UNKNOWN;
    }

    return result;
}

bool RequestHandler::IsApiRequest(const std::vector<std::string>& splittedRequest) {
    if (!splittedRequest.empty() && splittedRequest.front() == "api") {
        return true;
    }
    return false;
}

std::string RequestHandler::DetectFileExtension(const std::string& fileName) {
    std::string result;

    auto extPos = fileName.find_last_of(".");
    if (extPos == std::string::npos) {
        return result;
    }
    result = fileName.substr(extPos + 1);
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    
    return result;
}

}  // namespace http_handler
