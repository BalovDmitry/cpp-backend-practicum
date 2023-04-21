#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "http_server.h"
#include "path_helper.h"
#include "game.h"
#include "request_handler_strategy.h"
#include "request_handler_helper.h"
#include "command_line_parser.h"

#include <boost/asio/strand.hpp>
#include <boost/json/parse.hpp>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <cassert>
#include <mutex>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using namespace std::literals;

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    explicit RequestHandler(model::Game& game, net::io_context& ioc, const command_line::Args& args)
        : game_{game} 
        , strand_(net::make_strand(ioc)) 
        , args_(args) {

        strategy_api_ = std::make_shared<RequestHandlerStrategyApi>(game_, strand_, args_.randomize_spawn_point, args_.tick_period);
        strategy_static_ = std::make_shared<RequestHandlerStrategyStaticFile>(fs::weakly_canonical(args_.source_dir));
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (IsApiRequest(req)) {
            return net::dispatch(strand_, [self = this->shared_from_this(), req = std::move(req), send = std::move(send)] {
                //self->SetHandleStrategy(self->strategy_api_);
                send(self->strategy_api_->HandleRequest(std::decay_t<decltype(req)>(req)));
            }); 
        } else {
            //SetHandleStrategy(strategy_static_);
            send(strategy_static_->HandleRequest(std::move(req)));
        }
    }

// private:
//     void SetHandleStrategy(std::shared_ptr<RequestHandlerStrategyIntf> strategy) { strategy_ = strategy; }

private:
    model::Game& game_;
    const command_line::Args& args_;
    net::strand<net::io_context::executor_type> strand_;

    //std::shared_ptr<RequestHandlerStrategyIntf> strategy_;
    std::shared_ptr<RequestHandlerStrategyIntf> strategy_api_;
    std::shared_ptr<RequestHandlerStrategyIntf> strategy_static_;
};


}  // namespace http_handler
