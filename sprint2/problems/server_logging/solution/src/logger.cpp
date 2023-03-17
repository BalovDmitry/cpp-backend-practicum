#include "logger.h"

#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр 
#include <boost/json/serialize.hpp>

namespace logger {

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(file, "File", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(line, "Line", int) 
BOOST_LOG_ATTRIBUTE_KEYWORD(json_data, "JsonData", boost::json::object)

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    boost::json::object val;

    val["timestamp"] = to_iso_extended_string(*rec[timestamp]);
    val["data"] = *rec[json_data];
    val["message"] = *rec[expr::smessage];

    strm << boost::json::serialize(val);
}

void InitBoostLogFilter() {
    logging::add_common_attributes();

    logging::add_console_log(
        std::cout,
        keywords::format = &MyFormatter
    );
}

void LogJsonAndMessage(boost::json::object val, std::string_view message) {
    BOOST_LOG_TRIVIAL(info) << logging::add_value(json_data, val)
                            << message;
}

void LogMessage(std::string_view message) {

}

void LogJson(boost::json::value val) {

}

}