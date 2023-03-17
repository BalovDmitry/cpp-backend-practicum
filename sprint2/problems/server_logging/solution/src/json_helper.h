#pragma once

#include <boost/json/parse.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <optional>

#include "model.h"

namespace json_helper {

namespace net = boost::asio;
using tcp = net::ip::tcp;

bool AddRoadsToMap(model::Map& currentMap, const boost::json::value& mapObj);
bool AddBuildingsToMap(model::Map& currentMap, const boost::json::value& mapObj);
bool AddOfficesToMap(model::Map& currentMap, const boost::json::value& mapObj);

boost::json::array CreateRoadsArray(const model::Map& map);
boost::json::array CreateBuildingsArray(const model::Map& map);
boost::json::array CreateOfficesArray(const model::Map& map);

template<typename CodeType, typename MessageType> 
boost::json::object CreateErrorValue(CodeType&& code, MessageType&& message) {
    boost::json::object val;

    val["code"] = std::forward<CodeType>(code);
    val["message"] = std::forward<MessageType>(message);

    return val;
}

boost::json::object CreateStartServerValue(net::ip::port_type port, const net::ip::address& address);
boost::json::object CreateStopServerValue(uint8_t code, std::optional<std::string> exception = {});
boost::json::object CreateNetworkErrorValue(int code, const std::string& text, const std::string& where);
boost::json::object CreateRequestValue(const tcp::endpoint& endpoint, const std::string& uri, const std::string& method);
boost::json::object CreateResponseValue(int response_time, int code, const std::string& content_type);

}
