#include "request_handler_strategy.h"
#include "json_helper.h"
#include "logger.h"

#include <boost/json/serialize.hpp>
#include <fstream>
#include <string>
#include <chrono>

namespace http_handler {

//! INTERFACE METHODS

StringResponse RequestHandlerStrategyIntf::HandleRequest(StringRequest &&req)
{
    const auto text_response = [this, &req](http::status status, std::string_view text, std::string_view contentType) {
        return this->MakeStringResponse(status, text, req.version(), req.keep_alive(), contentType);
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    http::status status;
    std::string body;
    std::string_view contentType;

    // Non-virtual interface idiom
    HandleRequestImpl(req, status, body, contentType);
    
    auto response =  text_response(status, body, contentType);
    
    auto end = std::chrono::high_resolution_clock::now();
    logger::LogJsonAndMessage(json_helper::CreateResponseValue(
        std::chrono::duration<double, std::milli>(end - start).count(), static_cast<int>(status), std::string(contentType)), "response sent");

    return response;
}

StringResponse RequestHandlerStrategyIntf::MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive, std::string_view content_type)
{
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

bool RequestHandlerStrategyIntf::MakeBadRequestBody(std::string &bodyText, http::status &status)
{
    bodyText += boost::json::serialize(json_helper::CreateErrorValue("badRequest", "Bad request"));
    status = http::status::bad_request;
    return true;
}

bool RequestHandlerStrategyIntf::MakeMethodNotAllowedBody(std::string &bodyText, http::status &status)
{
    bodyText += boost::json::serialize(json_helper::CreateErrorValue("methodNotAllowed", "Method not allowed"));
    status = http::status::method_not_allowed;
    return true;
}

//! API HANDLER METHODS

void RequestHandlerStrategyApi::HandleRequestImpl(const StringRequest& req, http::status &status, std::string& body, std::string_view &content_type)
{
    if (req.method() == http::verb::get) {
        auto splittedRequest = GetVectorFromTarget(req.target());
        
        if (IsApiRequest(req)) {
            SetResponseData(splittedRequest, GetRequestType(splittedRequest), body, status);
            content_type = ContentType::APP_JSON;
        } else {
            //SetResponseDataStaticFile(splittedRequest, GetRequestType(splittedRequest), body, content_type, status);
        }
    
    } else if (req.method() == http::verb::head) {
        //!TODO: create body for HEAD
    } else {
        MakeMethodNotAllowedBody(body, status);
    }
}

void RequestHandlerStrategyApi::SetResponseData(const std::vector<std::string> &splittedRequest, RequestType requestType, std::string &bodyText, http::status &status)
{
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

RequestHandlerStrategyApi::RequestType RequestHandlerStrategyApi::GetRequestType(const std::vector<std::string> &splittedRequest)
{
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

bool RequestHandlerStrategyApi::CheckRequestCorrectness(const std::vector<std::string> &splittedRequest)
{
    if ((splittedRequest.size() == RequestTypeApiSize::GET_MAP_BY_ID || splittedRequest.size() == RequestTypeApiSize::GET_MAP_LIST)
        && splittedRequest[0] == "api"
        && splittedRequest[1] == "v1"
        && splittedRequest[2] == "maps") {
        
        return true;
    }
    return false;
}

bool RequestHandlerStrategyApi::MakeGetMapListBody(std::string &bodyText, http::status &status)
{
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

bool RequestHandlerStrategyApi::MakeGetMapByIdBody(model::Map::Id id, std::string &bodyText, http::status &status)
{
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

//! STATIC FILE HANDLER METHODS

void RequestHandlerStrategyStaticFile::HandleRequestImpl(const StringRequest& req, http::status &status, std::string& body, std::string_view &content_type)
{
}

void RequestHandlerStrategyStaticFile::SetResponseData(const std::vector<std::string> &splittedRequest, std::string &bodyText, http::status &status, std::string_view &contentType)
{
    auto relPath = path_helper::GetRelPathFromRequest(splittedRequest);
    auto absPath = path_helper::GetAbsPath(basePath_, relPath);

    std::ifstream fStream(absPath);
    if (fStream) {
        if (!path_helper::IsSubPath(absPath, basePath_)) {
            MakeBadRequestBody(bodyText, status);
            contentType = ContentType::TEXT_PLAIN;
            return;
        }

        std::stringstream buffer;
        buffer << fStream.rdbuf();
        bodyText += buffer.str();
        contentType = GetContentType(splittedRequest);
        status = http::status::ok;
    } else {
        MakeFileNotFoundBody(bodyText, status);
        contentType = ContentType::TEXT_PLAIN;
    }
}

bool RequestHandlerStrategyStaticFile::MakeFileNotFoundBody(std::string &bodyText, http::status &status)
{
    bodyText += boost::json::serialize(json_helper::CreateErrorValue("fileNotFound", "File not  found"));
    status = http::status::not_found;
    return true;
}

std::string_view RequestHandlerStrategyStaticFile::GetContentType(const std::vector<std::string> &splittedRequest)
{
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

std::string RequestHandlerStrategyStaticFile::DetectFileExtension(const std::string &fileName)
{
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

}