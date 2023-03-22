#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "http_server.h"
#include "path_helper.h"
#include "model.h"
#include "request_handler_strategy.h"
#include "request_handler_helper.h"

#include <vector>
#include <string>
#include <boost/json/parse.hpp>
#include <unordered_map>
#include <memory>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;

using namespace std::literals;

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
        if (IsApiRequest(req)) {
            SetHandleStrategy(std::make_shared<RequestHandlerStrategyApi>(game_));
        } else {
            SetHandleStrategy(std::make_shared<RequestHandlerStrategyStaticFile>(basePath_));
        }
        send(strategy_->HandleRequest(std::move(req)));
    }

private:
    void SetHandleStrategy(std::shared_ptr<RequestHandlerStrategyIntf> strategy) { strategy_ = strategy; }

private:
    model::Game& game_;
    std::filesystem::path basePath_;
    std::shared_ptr<RequestHandlerStrategyIntf> strategy_;
};


}  // namespace http_handler
