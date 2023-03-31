#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "game.h"
#include "path_helper.h"
#include "http_server.h"
#include "request_handler_helper.h"
#include "model_utils.h"

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

using namespace std::literals;

class RequestHandlerStrategyIntf {
public:
    StringResponse HandleRequest(StringRequest&& req);
    virtual ~RequestHandlerStrategyIntf() = default;

protected:
    virtual StringResponse HandleRequestImpl(
        StringRequest&& req, 
        http::status& status, 
        std::string& body, 
        std::string_view& content_type) = 0;

protected:
    bool MakeBadRequestBody(
        std::string& body, 
        http::status& status,
        const std::string& code = "badRequest",
        const std::string& message = "Method not allowed");
    bool MakeMethodNotAllowedBody(
        std::string& body, 
        http::status& status, 
        const std::string& code = "methodNotAllowed",
        const std::string& message = "Method not allowed");
};

class RequestHandlerStrategyApi : public RequestHandlerStrategyIntf {
public:
    RequestHandlerStrategyApi(model::Game& game)
        : game_(game) {}

    struct RequestTypeSize {
        RequestTypeSize() = delete;
        constexpr static size_t GET_MAP_LIST = 3;
        constexpr static size_t GET_MAP_BY_ID = 4;
        constexpr static size_t GET_PLAYERS_ON_MAP = 4;
        constexpr static size_t GET_GAME_STATE = 4;
        constexpr static size_t JOIN_GAME = 4;
        constexpr static size_t UPDATE_TIME = 4;
        constexpr static size_t MOVE_PLAYER = 5;
    };

    enum class RequestType {
        GET_MAP_LIST,
        GET_MAP_BY_ID,
        GET_PLAYERS_ON_MAP,
        GET_GAME_STATE,
        JOIN_GAME,
        MOVE_PLAYER,
        UPDATE_TIME,
        UNKNOWN
    };

protected:
    StringResponse HandleRequestImpl(
        StringRequest&& req, 
        http::status& status, 
        std::string& body, 
        std::string_view& content_type) override;

private:
    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                            bool keep_alive, RequestType request_type,
                            std::string_view content_type = ContentType::APP_JSON);
    void SetResponseDataGet(
        const StringRequest& req, 
        RequestType request_type, 
        std::string& body,
        http::status& status);
    void SetResponseDataPost(
        const StringRequest& req, 
        RequestType request_type, 
        std::string &body, 
        http::status &status);
    RequestType GetRequestType(const std::vector<std::string>& splitted_request);
    bool CheckRequestCorrectness(const std::vector<std::string>& splitted_request);
    
    // Get responses
    bool MakeGetMapListBody(std::string& body, http::status& status);
    bool MakeGetMapByIdBody(model::Map::Id id, std::string& body, http::status& status);
    bool MakeGetPlayersOnMapBody(const StringRequest& req, std::string& body, http::status& status);
    bool MakeGetGameStateBody(const StringRequest& req, std::string& body, http::status& status);
    
    // Post responses
    bool MakeJoinGameBody(std::string_view request, std::string& body, http::status& status);
    bool MakeMovePlayerBody(const StringRequest& req, std::string& body, http::status& status);
    bool MakeUpdateTimeBody(const StringRequest& req, std::string& body, http::status& status);

    std::string_view ReceiveTokenFromRequest(const StringRequest& req);
    model::Direction ReceiveDirectionFromRequest(const StringRequest& req);
    int ReceiveTimeFromRequest(const StringRequest& req);

private:
    model::Game& game_;
};

class RequestHandlerStrategyStaticFile : public RequestHandlerStrategyIntf {
public:
    RequestHandlerStrategyStaticFile(const std::filesystem::path& basePath)
        : basePath_(basePath) {}

protected:
    StringResponse HandleRequestImpl(
        StringRequest&& req, 
        http::status& status, 
        std::string& body, 
        std::string_view& content_type) override;

private:
    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                bool keep_alive,
                                std::string_view content_type);
    void SetResponseData(
        const std::vector<std::string>& splittedRequest,
        std::string& bodyText,
        http::status& status,
        std::string_view& contentType);
    bool MakeFileNotFoundBody(std::string& bodyText, http::status& status);
    std::string_view GetContentType(const std::vector<std::string>& splittedRequest);
    std::string DetectFileExtension(const std::string& fileName);

private:
    std::filesystem::path basePath_;
};

}