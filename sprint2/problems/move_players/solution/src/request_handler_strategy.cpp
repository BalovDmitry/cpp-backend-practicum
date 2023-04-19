#include "request_handler_strategy.h"
#include "json_helper.h"
#include "logger.h"

#include <boost/json/serialize.hpp>
#include <fstream>
#include <string>
#include <chrono>

#define BOOST_BEAST_USE_STD_STRING_VIEW

namespace http_handler {

//! INTERFACE METHODS

StringResponse RequestHandlerStrategyIntf::HandleRequest(StringRequest &&req)
{   
    auto start = std::chrono::high_resolution_clock::now();
    
    http::status status;
    std::string body;
    std::string_view contentType;

    // Non-virtual interface idiom
    auto response = HandleRequestImpl(std::move(req), status, body, contentType);

    auto end = std::chrono::high_resolution_clock::now();
    logger::LogJsonAndMessage(json_helper::CreateResponseValue(
        std::chrono::duration<double, std::milli>(end - start).count(), static_cast<int>(status), std::string(contentType)), "response sent");

    return response;
}

bool RequestHandlerStrategyIntf::MakeBadRequestBody(std::string &bodyText, http::status &status, const std::string &code, const std::string &message)
{
    bodyText += boost::json::serialize(json_helper::CreateErrorValue(code, message));
    status = http::status::bad_request;
    return true;
}

bool RequestHandlerStrategyIntf::MakeMethodNotAllowedBody(std::string &bodyText, http::status &status, const std::string &code, const std::string &message)
{
    bodyText += boost::json::serialize(json_helper::CreateErrorValue(code, message));
    status = http::status::method_not_allowed;
    return true;
}

//! API HANDLER METHODS

StringResponse RequestHandlerStrategyApi::HandleRequestImpl(StringRequest&& req, http::status &status, std::string& body, std::string_view &content_type)
{
    const auto text_response = [this, &req](http::status status, std::string_view text, RequestType type, std::string_view content_type) {
        return this->MakeStringResponse(status, text, req.version(), req.keep_alive(), type, content_type);
    };

    content_type = ContentType::APP_JSON;
    auto request_type = GetRequestType(GetVectorFromTarget(std::string_view(req.target().data(), req.target().size())));
    switch (request_type) {
        case RequestType::GET_MAP_BY_ID:
        case RequestType::GET_MAP_LIST: {
            if (req.method() == http::verb::get || req.method() == http::verb::head) {
                SetResponseDataGet(req, request_type, body, status);
            } else {
                MakeMethodNotAllowedBody(body, status);
            }
            break;
        }
        
        case RequestType::GET_PLAYERS_ON_MAP: {
            if (req.method() == http::verb::get || req.method() == http::verb::head) {
                SetResponseDataGet(req, request_type, body, status);
            } else {
                MakeMethodNotAllowedBody(body, status, "invalidMethod", "Only GET, HEAD methods are expected");
            }
            break;
        }

        case RequestType::GET_GAME_STATE: {
            if (req.method() == http::verb::get || req.method() == http::verb::head) {
                SetResponseDataGet(req, request_type, body, status);
            } else {
                MakeMethodNotAllowedBody(body, status, "invalidMethod", "Invalid method");
            }
            break;
        }

        case RequestType::JOIN_GAME: {
            if (req.method() == http::verb::post) {
                SetResponseDataPost(req, request_type, body, status);
            } else {
                MakeMethodNotAllowedBody(body, status, "invalidMethod", "Only POST method is expected");
            }
            break;
        }

        case RequestType::MOVE_PLAYER: {
            if (req.method() == http::verb::post) {
                SetResponseDataPost(req, request_type, body, status);
            } else {
                MakeMethodNotAllowedBody(body, status, "invalidMethod", "Only POST method is expected");
            }
            break;
        }

        default: {
            std::cout << "Default case" << std::endl;
            MakeBadRequestBody(body, status);
            break;
        }
    }

    return text_response(status, body, request_type, content_type);
}

StringResponse RequestHandlerStrategyApi::MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive, RequestType request_type, std::string_view content_type)
{
    StringResponse response(status, http_version);
    if (status == http::status::method_not_allowed && request_type == RequestType::JOIN_GAME) {
        response.set(http::field::allow, "POST");
    } else if (status == http::status::method_not_allowed && (request_type == RequestType::GET_PLAYERS_ON_MAP || request_type == RequestType::GET_GAME_STATE)) {
        response.set(http::field::allow, "GET, HEAD");
    }
    response.set(http::field::content_type, std::string(content_type));
    response.body() = std::string(body);
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    response.set(http::field::cache_control, "no-cache");
    return response;
}

void RequestHandlerStrategyApi::SetResponseDataGet(const StringRequest& req, RequestType request_type, std::string &body, http::status &status)
{
    switch (request_type) {
        case RequestType::GET_MAP_LIST: {
            MakeGetMapListBody(body, status);
            break;
        }

        case RequestType::GET_MAP_BY_ID: {
            MakeGetMapByIdBody(model::Map::Id(GetVectorFromTarget(std::string_view(req.target())).back()), body, status);
            break;
        }
        
        case RequestType::GET_PLAYERS_ON_MAP: {
            MakeGetPlayersOnMapBody(req, body, status);
            break;
        }

        case RequestType::GET_GAME_STATE: {
            MakeGetGameStateBody(req, body, status);
            break;
        }

        case RequestType::UNKNOWN: {
            std::cout << "Unknown request type" << std::endl;
            MakeBadRequestBody(body, status);
            break;
        }

        default:
            break;
    }
}

void RequestHandlerStrategyApi::SetResponseDataPost(const StringRequest& req, RequestType requestType, std::string &body, http::status &status)
{
    switch (requestType) {
        case RequestType::JOIN_GAME: {
            MakeJoinGameBody(req.body(), body, status);
            break;
        }

        case RequestType::MOVE_PLAYER: {
            MakeMovePlayerBody(req, body, status);
            break;
        }

        case RequestType::UNKNOWN: {
            std::cout << "Unknown set response data post" << std::endl;
            MakeBadRequestBody(body, status);
            break;
        }

        default:
            break;
    }
}

RequestHandlerStrategyApi::RequestType RequestHandlerStrategyApi::GetRequestType(const std::vector<std::string> &splittedRequest)
{
    RequestType result = RequestType::UNKNOWN;
    bool is_req_correct = CheckRequestCorrectness(splittedRequest);

    if (is_req_correct) {
        if (splittedRequest.size() == RequestTypeSize::GET_MAP_LIST) {
            result = RequestType::GET_MAP_LIST;
        } else if (splittedRequest.size() == RequestTypeSize::GET_MAP_BY_ID && splittedRequest.back().starts_with("map")) {
            result = RequestType::GET_MAP_BY_ID;
        } else if (splittedRequest.size() == RequestTypeSize::GET_PLAYERS_ON_MAP && splittedRequest.back() == "players") {
            result = RequestType::GET_PLAYERS_ON_MAP;
        } else if (splittedRequest.size() == RequestTypeSize::GET_GAME_STATE && splittedRequest.back() == "state") {
            result = RequestType::GET_GAME_STATE;
        } else if (splittedRequest.size() == RequestTypeSize::JOIN_GAME && splittedRequest.back() == "join") {
            result = RequestType::JOIN_GAME;
        } else if (splittedRequest.size() == RequestTypeSize::MOVE_PLAYER && splittedRequest.back() == "action") {
            result = RequestType::MOVE_PLAYER;
        }
    }

    return result;
}

bool RequestHandlerStrategyApi::CheckRequestCorrectness(const std::vector<std::string> &splittedRequest)
{
    if ((splittedRequest.size() == RequestTypeSize::GET_MAP_BY_ID 
        || splittedRequest.size() == RequestTypeSize::GET_MAP_LIST
        || splittedRequest.size() == RequestTypeSize::GET_PLAYERS_ON_MAP
        || splittedRequest.size() == RequestTypeSize::GET_GAME_STATE
        || splittedRequest.size() == RequestTypeSize::JOIN_GAME
        || splittedRequest.size() == RequestTypeSize::MOVE_PLAYER)
        && splittedRequest[0] == "api"
        && splittedRequest[1] == "v1") {
        
        return true;
    }
    return false;
}

std::string_view RequestHandlerStrategyApi::ReceiveTokenFromRequest(const StringRequest &req)
{
    std::string_view result;

    auto it = req.find("authorization");
    if (it == req.end()) {
        throw std::invalid_argument(std::string(ErrorMessages::INVALID_TOKEN));
    }
    
    auto str = req.at("authorization");
    if (str.find("Bearer") != std::string::npos) {
        auto pos = str.find_last_of(' ');
        result = str.substr(pos + 1);
    } else {
        throw std::invalid_argument(std::string(ErrorMessages::INVALID_TOKEN));
    }

    if (result.empty()) {
        throw std::invalid_argument(std::string(ErrorMessages::INVALID_TOKEN));
    }

    return result;
}

model::Direction RequestHandlerStrategyApi::ReceiveDirectionFromRequest(const StringRequest &req)
{
    using namespace model;

    boost::json::value val = boost::json::parse(std::string(req.body()));
    auto move_val = val.as_object().at("move").as_string();

    if (move_val == "L") {
        return Direction::WEST;
    } else if (move_val == "R") {
        return Direction::EAST;
    } else if (move_val == "U") {
        return Direction::NORTH;
    } else if (move_val == "D") {
        return Direction::SOUTH;
    } else if (move_val.empty()) {
        return Direction::NO_DIRECTION;
    } else {
        throw std::invalid_argument(std::string(ErrorMessages::INVALID_ARGUMENT_DIRECTION));
    }
}

// Get responses

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

bool RequestHandlerStrategyApi::MakeGetPlayersOnMapBody(const StringRequest& req, std::string& body, http::status& status)
{
    boost::json::object res;

    try {
        auto token = ReceiveTokenFromRequest(req);
        auto player = game_.FindPlayerByToken(model::Token(std::string(token)));
        const auto map_id = player.GetMapId();
        const auto& players = game_.GetPlayersOnMap(map_id);
        for (const auto& player_id : players) {
            const auto& current_player = game_.FindPlayerById(player_id);
            boost::json::object name_obj;
            name_obj["name"] = current_player.GetName();
            res[std::to_string(current_player.GetId())] = name_obj;
        }
        status = http::status::ok;
    } catch (std::exception& e) {
        std::string message;
        std::string code;
        const std::string& what = e.what();

        if (what == ErrorMessages::INVALID_TOKEN) {
            code = what;
            message = "Authorization header is missing";
            status = http::status::unauthorized;
        } else if (what == ErrorMessages::UNKNOWN_TOKEN) {
            code = what;
            message = "Player token has not been found";
            status = http::status::unauthorized;
        }
        res = json_helper::CreateErrorValue(code, message);
    }

    body += boost::json::serialize(res);

    return true;
}

bool RequestHandlerStrategyApi::MakeGetGameStateBody(const StringRequest &req, std::string &body, http::status &status)
{
    boost::json::object res;

    try {
        auto token = ReceiveTokenFromRequest(req);
        
        // Just to ensure that token is valid
        game_.FindPlayerByToken(model::Token(std::string(token)));
        
        boost::json::object players_obj;
        for (const auto& player : game_.GetPlayers()) {
            boost::json::object temp;

            const auto& dog = player.GetDog();
            temp["pos"] = boost::json::array({ dog->GetPosition().x, dog->GetPosition().y });
            temp["speed"] = boost::json::array({ dog->GetSpeed().v_x, dog->GetSpeed().v_y });
            //temp["speed"] = boost::json::array({ 0, 0 });
            temp["dir"] = dog->GetDirectionString();

            players_obj[std::to_string(player.GetId())] = temp;
        }

        res["players"] = players_obj;
        status = http::status::ok;
    } catch (std::exception& e) {
        std::string message;
        std::string code;
        const std::string& what = e.what();

        if (what == ErrorMessages::INVALID_TOKEN) {
            code = what;
            message = "Authorization header is missing";
            status = http::status::unauthorized;
        } else if (what == ErrorMessages::UNKNOWN_TOKEN) {
            code = what;
            message = "Player token has not been found";
            status = http::status::unauthorized;
        }
        res = json_helper::CreateErrorValue(code, message);
    }

    body += boost::json::serialize(res);

    return true;
}

// Post responses

bool RequestHandlerStrategyApi::MakeJoinGameBody(std::string_view request, std::string &body, http::status &status)
{
    boost::json::object res;

    try {
        boost::json::value val = boost::json::parse(std::string(request));
        if (!val.as_object().contains("userName") || !val.as_object().contains("mapId")) {
            throw std::invalid_argument(std::string(ErrorMessages::INVALID_ARGUMENT_PARSE));
        }

        std::string name = val.as_object().at("userName").as_string().c_str();
        if (name.empty()) {
            throw std::invalid_argument(std::string(ErrorMessages::INVALID_ARGUMENT_NAME));
        }   
        std::string mapId = val.as_object().at("mapId").as_string().c_str();
        auto token = game_.JoinGame(name, model::Map::Id{mapId});
        auto player = game_.FindPlayerByToken(token);
        res["authToken"] = *token;
        res["playerId"] = player.GetId();
        status = http::status::ok;

    } catch (std::exception& e) {
        std::string message;
        std::string code;
        const std::string& what = e.what();

        if (what == ErrorMessages::INVALID_ARGUMENT_PARSE) {
            code = "invalidArgument";
            message = "Join game request parse error";
            status = http::status::bad_request;
        } else if (what == ErrorMessages::INVALID_ARGUMENT_NAME) {
            code = "invalidArgument";
            message = "Invalid name";
            status = http::status::bad_request;
        } else if (what == ErrorMessages::INVALID_ARGUMENT_MAP) {
            code = "mapNotFound";
            message = "Map not found";
            status = http::status::not_found;
        }

        res = json_helper::CreateErrorValue(code, message);
    }
    body += boost::json::serialize(res);

    return true;
}

bool RequestHandlerStrategyApi::MakeMovePlayerBody(const StringRequest& req, std::string &body, http::status &status)
{
    using namespace model;

    boost::json::object res;

    try {
        auto token = ReceiveTokenFromRequest(req);
        auto player = game_.FindPlayerByToken(model::Token(std::string(token)));
        auto direction = ReceiveDirectionFromRequest(req);
        player.SetDirection(direction);
        status = http::status::ok;
    } catch (std::exception& e) {
        std::string message;
        std::string code;
        const std::string& what = e.what();

        if (what == ErrorMessages::INVALID_TOKEN) {
            code = what;
            message = "Authorization header is missing";
            status = http::status::unauthorized;
        } else if (what == ErrorMessages::UNKNOWN_TOKEN) {
            code = what;
            message = "Player token has not been found";
            status = http::status::unauthorized;
        } else {
            MakeBadRequestBody(body, status, "invalidArgument", "Failed to parse action");
            return true;
        }
        res = json_helper::CreateErrorValue(code, message);
    }
    
    body += boost::json::serialize(res);

    return true;
}

//! STATIC FILE HANDLER METHODS

StringResponse RequestHandlerStrategyStaticFile::HandleRequestImpl(StringRequest&& req, http::status &status, std::string& body, std::string_view &content_type)
{
    const auto text_response = [this, &req](http::status status, std::string_view text, std::string_view content_type) {
        return this->MakeStringResponse(status, text, req.version(), req.keep_alive(), content_type);
    };

    if (req.method() == http::verb::get || req.method() == http::verb::head) {
        // auto splittedRequest = GetVectorFromTarget(req.target());
        // SetResponseData(splittedRequest, body, status, content_type);

        SetResponseData(std::string_view(req.target()), body, status, content_type);
    } else {
        MakeMethodNotAllowedBody(body, status);
    }

    return text_response(status, body, content_type);
}

StringResponse RequestHandlerStrategyStaticFile::MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive, std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

void RequestHandlerStrategyStaticFile::SetResponseData(const std::string_view &request, std::string &body, http::status &status, std::string_view &content_type) {
    auto decoded_path = path_helper::GetDecodedPath(request);
    auto abs_path = path_helper::GetAbsPath(basePath_, decoded_path);

    //std::cout << "Req target: " << request << std::endl;
    // std::cout << "Base path: " << basePath_ << std::endl;
    // std::cout << "Decoded path: " << decoded_path << std::endl;
    //std::cout << "Abs path: " << abs_path << std::endl;
    std::ifstream fstream(abs_path);
    if (fstream) {
        if (!path_helper::IsSubPath(abs_path, basePath_)) {
            std::cout << "Abs path: " << abs_path <<  " is not a subpath of base:" << basePath_ << std::endl;
            std::cout << "Decoded path: " << decoded_path << std::endl;
            MakeBadRequestBody(body, status);
            content_type = ContentType::TEXT_PLAIN;
            return;
        }

        std::stringstream buffer;
        buffer << fstream.rdbuf();
        body += buffer.str();
        content_type = GetContentType(request);
        status = http::status::ok;
    }  else {
        std::cout << "Abs path: " << abs_path << " doesn't exist!" << std::endl;
        MakeFileNotFoundBody(body, status);
        content_type = ContentType::TEXT_PLAIN;
    }
}

// void RequestHandlerStrategyStaticFile::SetResponseData(const std::vector<std::string> &splittedRequest, std::string &bodyText, http::status &status, std::string_view &contentType) {
//     auto relPath = path_helper::GetRelPathFromRequest(splittedRequest);
//     auto absPath = path_helper::GetAbsPath(basePath_, relPath);

//     std::ifstream fStream(absPath);
//     if (fStream) {
//         if (!path_helper::IsSubPath(absPath, basePath_)) {
//             MakeBadRequestBody(bodyText, status);
//             contentType = ContentType::TEXT_PLAIN;
//             return;
//         }

//         std::stringstream buffer;
//         buffer << fStream.rdbuf();
//         bodyText += buffer.str();
//         contentType = GetContentType(splittedRequest);
//         status = http::status::ok;
//     } else {
//         MakeFileNotFoundBody(bodyText, status);
//         contentType = ContentType::TEXT_PLAIN;
//     }
// }

bool RequestHandlerStrategyStaticFile::MakeFileNotFoundBody(std::string &bodyText, http::status &status)
{
    bodyText += boost::json::serialize(json_helper::CreateErrorValue("fileNotFound", "File not  found"));
    status = http::status::not_found;
    return true;
}

std::string_view RequestHandlerStrategyStaticFile::GetContentType(const std::string_view &request) {
    std::string_view result = ContentType::UNKNOWN;

    auto splitted_request = GetVectorFromTarget(request);

    if (splitted_request.empty()) {
        return ContentType::TEXT_HTML;
    }

    auto ext = DetectFileExtension(splitted_request.back());
    if (ExtensionToContentType.contains(ext)) {
        result = ExtensionToContentType.at(ext);
    } else {
        result = ContentType::UNKNOWN;
    }

    return result;
}

// std::string_view RequestHandlerStrategyStaticFile::GetContentType(const std::vector<std::string> &splittedRequest)
// {
//     std::string_view result = ContentType::UNKNOWN;

//     if (splittedRequest.empty()) {
//         return ContentType::TEXT_HTML;
//     }

//     auto ext = DetectFileExtension(splittedRequest.back());
//     if (ExtensionToContentType.contains(ext)) {
//         result = ExtensionToContentType.at(ext);
//     } else {
//         result = ContentType::UNKNOWN;
//     }

//     return result;
// }

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