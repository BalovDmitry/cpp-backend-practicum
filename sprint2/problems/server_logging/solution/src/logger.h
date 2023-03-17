#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>        // для logging::core
#include <boost/date_time.hpp>
#include <boost/json/parse.hpp>
#include <string_view>

namespace logger {

namespace logging = boost::log;

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm);
void InitBoostLogFilter();
void LogMessage(std::string_view message);
void LogJson(boost::json::value val);
void LogJsonAndMessage(boost::json::object val, std::string_view message);

}