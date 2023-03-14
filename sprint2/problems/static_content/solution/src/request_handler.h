#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "http_server.h"
#include "path_helper.h"
#include "model.h"

#include <vector>
#include <string>
#include <boost/json/parse.hpp>
#include <unordered_map>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view APP_JSON = "application/json"sv;
    constexpr static std::string_view APP_XML = "application/xml"sv;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view TEXT_CSS = "text/css"sv;
    constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
    constexpr static std::string_view TEXT_JS = "text/javascript"sv;
    constexpr static std::string_view IMAGE_PNG = "image/png"sv;
    constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
    constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
    constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
    constexpr static std::string_view IMAGE_ICO = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
    constexpr static std::string_view IMAGE_SVG = "image/svg+xml"sv;
    constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;
    constexpr static std::string_view UNKNOWN = "application/octet-stream"sv;
};

static const std::unordered_map<std::string, std::string_view> ExtensionToContentType {
      { "json", ContentType::APP_JSON }
    , { "xml", ContentType::APP_XML }
    , { "html", ContentType::TEXT_HTML }
    , { "css", ContentType::TEXT_CSS }
    , { "plain", ContentType::TEXT_PLAIN }
    , { "javascript", ContentType::TEXT_JS }
    , { "png", ContentType::IMAGE_PNG }
    , { "jpg", ContentType::IMAGE_JPEG }
    , { "jpe", ContentType::IMAGE_JPEG }
    , { "jpeg", ContentType::IMAGE_JPEG }
    , { "gif", ContentType::IMAGE_GIF }
    , { "bmp", ContentType::IMAGE_BMP }
    , { "ico", ContentType::IMAGE_ICO }
    , { "tif", ContentType::IMAGE_TIFF }
    , { "tiff", ContentType::IMAGE_TIFF }
    , { "svg", ContentType::IMAGE_SVG }
    , { "svgz", ContentType::IMAGE_SVG }
    , { "mp3", ContentType::AUDIO_MPEG }
};

enum class RequestTypeApi {
    GET_MAP_LIST,
    GET_MAP_BY_ID,
    UNKNOWN
};

struct RequestTypeApiSize {
    RequestTypeApiSize() = delete;
    constexpr static size_t GET_MAP_LIST = 3;
    constexpr static size_t GET_MAP_BY_ID = 4;
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, const std::filesystem::path& basePath = "/home/")
        : game_{game} {

        basePath_ = fs::weakly_canonical(basePath);
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
    bool CheckRequestCorrectnessApi(const std::vector<std::string>& splittedRequest);
    RequestTypeApi GetRequestTypeApi(const std::vector<std::string>& splittedRequest);
    std::string_view GetContentType(const std::vector<std::string>& splittedRequest);
    void MakeBodyTextApi(
        const std::vector<std::string>& splittedRequest, 
        RequestTypeApi requestType, 
        std::string& bodyText, 
        http::status& status);
    void SetResponseDataStaticFile(
        const std::vector<std::string>& splittedRequest, 
        RequestTypeApi requestType, 
        std::string& bodyText,
        std::string_view& contentType,
        http::status& status);


    bool IsApiRequest(const std::vector<std::string>& splittedRequest);

    bool MakeGetMapListBody(std::string& bodyText, http::status& status);
    bool MakeGetMapByIdBody(model::Map::Id id, std::string& bodyText, http::status& status);
    bool MakeBadRequestBody(std::string& bodyText, http::status& status);
    bool MakeMethodNotAllowedBody(std::string& bodyText, http::status& status);
    bool MakeFileNotFoundBody(std::string& bodyText, http::status& status);
    std::string DetectFileExtension(const std::string& fileName);

private:
    model::Game& game_;
    std::filesystem::path basePath_;
};

}  // namespace http_handler
