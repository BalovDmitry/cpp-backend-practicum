#include "ticker.h"

#include <iostream>

namespace http_handler {

void Ticker::Start() {
    last_tick_ = std::chrono::steady_clock::now();
    ScheduleTick();
}

void Ticker::ScheduleTick() {
    timer_.expires_after(std::chrono::duration_cast<std::chrono::milliseconds>(period_));
    timer_.async_wait(
        net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
            self->OnTick(ec);
    }));
}

void Ticker::OnTick(sys::error_code ec) {
    auto current_tick = std::chrono::steady_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - last_tick_);
    std::cout << "On tick: " << t.count() << std::endl;
    handler_(std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - last_tick_));
    last_tick_ = current_tick;
    ScheduleTick();
}

}