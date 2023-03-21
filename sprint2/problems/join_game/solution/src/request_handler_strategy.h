#pragma once

#include "request_handler.h"
#include "model.h"
#include "path_helper.h"

namespace http_handler {

class RequestHandlerStrategyIntf {
public:
    StringResponse HandleRequest(StringRequest&& req);

protected:
    virtual void HandleRequestImpl(
        const StringRequest& req, 
        http::status& status, 
        std::string& body, 
        std::string_view& content_type) = 0;

protected:
    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                bool keep_alive,
                                std::string_view content_type = ContentType::APP_JSON);
    bool MakeBadRequestBody(std::string& bodyText, http::status& status);
    bool MakeMethodNotAllowedBody(std::string& bodyText, http::status& status);


};

class RequestHandlerStrategyApi : public RequestHandlerStrategyIntf {
public:
    RequestHandlerStrategyApi(model::Game& game)
        : game_(game) {}

    struct RequestTypeSize {
        RequestTypeSize() = delete;
        constexpr static size_t GET_MAP_LIST = 3;
        constexpr static size_t GET_MAP_BY_ID = 4;
    };

    enum class RequestType {
        GET_MAP_LIST,
        GET_MAP_BY_ID,
        UNKNOWN
    };

protected:
    void HandleRequestImpl(
        const StringRequest& req, 
        http::status& status, 
        std::string& body, 
        std::string_view& content_type) override;

private:
    void SetResponseData(
        const std::vector<std::string>& splittedRequest, 
        RequestType requestType, 
        std::string& bodyText,
        http::status& status);
    RequestType GetRequestType(const std::vector<std::string>& splittedRequest);
    bool CheckRequestCorrectness(const std::vector<std::string>& splittedRequest);
    bool MakeGetMapListBody(std::string& bodyText, http::status& status);
    bool MakeGetMapByIdBody(model::Map::Id id, std::string& bodyText, http::status& status);

private:
    model::Game& game_;
};

class RequestHandlerStrategyStaticFile : public RequestHandlerStrategyIntf {
public:
    RequestHandlerStrategyStaticFile(const std::filesystem::path& basePath)
        : basePath_(basePath) {}

protected:
    void HandleRequestImpl(
        const StringRequest& req, 
        http::status& status, 
        std::string& body, 
        std::string_view& content_type) override;

private:
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