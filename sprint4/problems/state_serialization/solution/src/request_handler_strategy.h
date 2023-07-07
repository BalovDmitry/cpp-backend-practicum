#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "game.h"
#include "path_helper.h"
#include "http_server.h"
#include "request_handler_helper.h"
#include "model_utils.h"
#include "ticker.h"
#include "loot_generator.h"

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;
namespace net = boost::asio;

using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;
using Strand = net::strand<net::io_context::executor_type>;

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

class RequestHandlerStrategyApi : public RequestHandlerStrategyIntf, public std::enable_shared_from_this<RequestHandlerStrategyApi> {
public:
    RequestHandlerStrategyApi(model::Game& game, 
                                Strand& strand, 
                                bool randomize_spawn_point = false, 
                                int tick_period = 0, 
                                const std::filesystem::path& state_file = "",
                                int save_state_period = 0);

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
    std::chrono::milliseconds ReceiveTimeFromRequest(const StringRequest& req);

private:
    void UpdateTimeInSessions(std::chrono::milliseconds delta = 1000ms);

private:
    model::Game& game_;
    Strand& strand_;
    std::chrono::milliseconds tick_period_;
    net::steady_timer timer_{strand_};
    std::shared_ptr<Ticker> ticker_;
    std::shared_ptr<loot_gen::LootGenerator> loot_generator_;
    bool ticker_started_;
    bool is_debug_mode_;
    bool randomize_spawn_point_;
    std::filesystem::path state_file_;
    std::chrono::milliseconds save_state_period_;
};

class RequestHandlerStrategyStaticFile : public RequestHandlerStrategyIntf {
public:
    RequestHandlerStrategyStaticFile(const std::filesystem::path& base_path)
        : base_path_(base_path) {}

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
        const std::string_view& request,
        std::string& body,
        http::status& status,
        std::string_view& content_type);
    std::string_view GetContentType(const std::string_view &request);

    bool MakeFileNotFoundBody(std::string& bodyText, http::status& status);
    std::string DetectFileExtension(const std::string& fileName);

private:
    std::filesystem::path base_path_;
};

}