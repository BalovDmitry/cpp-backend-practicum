#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "http_server.h"
#include "model.h"

#include <vector>
#include <string>
#include <boost/json/parse.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view APP_JSON = "application/json"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

enum class RequestType {
    GET_MAP_LIST,
    GET_MAP_BY_ID,
    UNKNOWN
};

struct RequestTypeSize {
    RequestTypeSize() = delete;
    constexpr static size_t GET_MAP_LIST = 3;
    constexpr static size_t GET_MAP_BY_ID = 4;
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        send(HandleRequest(std::move(req)));

    }

private:
    StringResponse HandleRequest(StringRequest&& req);
    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                    bool keep_alive,
                                    std::string_view content_type = ContentType::APP_JSON);
    std::vector<std::string> GetVectorFromTarget(std::string_view target);
    bool CheckRequestCorrectness(const std::vector<std::string>& splittedRequest);
    RequestType GetRequestType(const std::vector<std::string>& splittedRequest);
    void MakeBodyText(const std::vector<std::string>& splittedRequest, 
        RequestType requestType, 
        std::string& bodyText, 
        http::status& status);

    boost::json::object CreateErrorValue(const std::string& code, const std::string& message);

    boost::json::array CreateRoadsArray(const model::Map& map);
    boost::json::array CreateBuildingsArray(const model::Map& map);
    boost::json::array CreateOfficesArray(const model::Map& map);

    bool MakeGetMapListBody(std::string& bodyText, http::status& status);
    bool MakeGetMapByIdBody(model::Map::Id id, std::string& bodyText, http::status& status);
    bool MakeBadRequestBody(std::string& bodyText, http::status& status);
    bool MakeMethodNotAllowedBody(std::string& bodyText, http::status& status);

private:
    model::Game& game_;
};

}  // namespace http_handler
