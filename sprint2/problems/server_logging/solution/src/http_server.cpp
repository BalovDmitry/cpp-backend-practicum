#include "http_server.h"
#include "logger.h"
#include "json_helper.h"

namespace http_server {

void ReportError(beast::error_code ec, std::string_view what) {
    logger::LogJsonAndMessage(json_helper::CreateNetworkErrorValue(ec.value(), ec.message(), what), "error");
}

void ReportRequest(const tcp::endpoint& endpoint, std::string_view uri, std::string_view method) {
    logger::LogJsonAndMessage(json_helper::CreateRequestValue(endpoint, uri, method), "request received");
}

void SessionBase::Run() {
    // Вызываем метод Read, используя executor объекта stream_.
    // Таким образом вся работа со stream_ будет выполняться, используя его executor
    net::dispatch(stream_.get_executor(),
                    beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

void SessionBase::Read() {
    // Очищаем запрос от прежнего значения (метод Read может быть вызван несколько раз)
    request_ = {};
    stream_.expires_after(30s);
    // Считываем request_ из stream_, используя buffer_ для хранения считанных данных
    http::async_read(stream_, buffer_, request_,
                        // По окончании операции будет вызван метод OnRead
                        beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
}

void SessionBase::OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read) {
    if (ec == http::error::end_of_stream) {
        // Нормальная ситуация - клиент закрыл соединение
        return Close();
    }
    
    if (ec) {
        return ReportError(ec, "read"sv);
    } else {
        ReportRequest(stream_.socket().local_endpoint(), request_.target(), request_.method_string());
    }
    

    HandleRequest(std::move(request_));
}

void SessionBase::OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written) {
    if (ec) {
        return ReportError(ec, "write"sv);
    }

    if (close) {
        // Семантика ответа требует закрыть соединение
        return Close();
    }

    // Считываем следующий запрос
    Read();
}

void SessionBase::Close() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
}

}  // namespace http_server
