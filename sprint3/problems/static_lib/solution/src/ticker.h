#pragma once

#include <boost/asio/strand.hpp>
#include <boost/timer/timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <chrono>

namespace http_handler {

namespace net = boost::asio;
namespace sys = boost::system;

using namespace std::literals;

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
    using Handler = std::function<void(std::chrono::milliseconds)>;
    using Strand = net::strand<net::io_context::executor_type>;

    Ticker(Strand& strand, std::chrono::milliseconds period, Handler handler)
        : strand_(strand)
        , period_(period)
        , handler_(handler) {}

    void Start();

private:
    void ScheduleTick();
    void OnTick(sys::error_code ec);

private:
    Strand& strand_;
    net::steady_timer timer_{strand_};
    std::chrono::milliseconds period_;
    Handler handler_;
    std::chrono::time_point<std::chrono::steady_clock> last_tick_;
}; 

}