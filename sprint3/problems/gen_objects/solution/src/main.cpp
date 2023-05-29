#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>
#include <boost/asio/signal_set.hpp>

#include "json_loader.h"
#include "json_helper.h"
#include "request_handler.h"
#include "logger.h"
#include "command_line_parser.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = http_server::sys;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned workersCount, const Fn& fn) {
    workersCount = std::max(1u, workersCount);

    #ifdef __clang__
        std::vector<std::thread> workers;
    #else
        std::vector<std::jthread> workers;
    #endif

    workers.reserve(workersCount - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--workersCount) {
        workers.emplace_back(fn);
    }
    fn();

    #ifdef __clang__
        for (auto& worker : workers) {
            worker.join();
        }
    #endif
}

}  // namespace

int main(int argc, const char* argv[]) {
    try {
        // 0. Инициализируем логгер
        logger::InitBoostLogFilter();

        if (auto args = command_line::ParseCommandLine(argc, argv)) {
            // 1. Загружаем карту из файла и построить модель игры
            //std::string path = "/Users/balovdmitry/Desktop/SB/cpp-backend-practicum/sprint1/problems/map_json/precode/data/config.json";
            //model::Game game = json_loader::LoadGame(path);
            model::Game game = json_loader::LoadGame(args.value().config_file);

            // 2. Инициализируем io_context
            const unsigned numThreads = std::thread::hardware_concurrency();
            net::io_context ioc(numThreads);

            // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
            // Подписываемся на сигналы и при их получении завершаем работу сервера
            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signalNumber) {
                if (!ec) {
                    std::cout << "Signal "sv << signalNumber << " received"sv << std::endl;
                    ioc.stop();
                }
            });

            // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
            auto handler = std::make_shared<http_handler::RequestHandler>(game, ioc, args.value());

            // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
            const auto address = net::ip::make_address("127.0.0.1");
            constexpr net::ip::port_type port = 8080;
            http_server::ServeHttp(ioc, {address, port}, [&handler](auto&& req, auto&& send) {
                (*handler)(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
            });
            
            logger::LogJsonAndMessage(json_helper::CreateStartServerValue(port, address), "server has started");

            // 6. Запускаем обработку асинхронных операций
            RunWorkers(std::max(1u, numThreads), [&ioc] {
                ioc.run();
            });
            logger::LogJsonAndMessage(json_helper::CreateStopServerValue(EXIT_SUCCESS), "server exited");
        }
    } catch (const std::exception& ex) {
        logger::LogJsonAndMessage(json_helper::CreateStopServerValue(EXIT_FAILURE, ex.what()), "server exited");
        return EXIT_FAILURE;
    }
}
