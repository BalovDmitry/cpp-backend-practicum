#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <string>
#include <unordered_map>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;

bool IsApiRequest(const StringRequest& req);
std::vector<std::string> GetVectorFromTarget(std::string_view target);

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
    , { "js", ContentType::TEXT_JS }
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


}