#include "request_handler_helper.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

#include "logger.h"
#include "json_helper.h"

namespace http_handler {

bool IsApiRequest(const StringRequest &req)
{
    logger::LogJsonAndMessage({}, "in IsApiRequest");
    std::string_view t = {req.target().data(), req.target().size()};
    boost::json::object val;
    val["string view request"] = std::string(t);
    logger::LogJsonAndMessage(val, "after casting to string view");
    auto splittedRequest = GetVectorFromTarget(t);
    logger::LogJsonAndMessage({}, "after get splitted request");

    if (!splittedRequest.empty() && splittedRequest.front() == "api") {
        return true;
    }
    return false;
}

std::vector<std::string> GetVectorFromTarget(std::string_view target)
{
    std::vector<std::string> result;
    boost::json::object val;
    val["target"] = target;
    logger::LogJsonAndMessage(val, "in GetVectorFromTarget");
    try {
        if (target.size() > 1) {
            boost::split(result, std::string_view(target.data() + 1, target.size() - 1), boost::is_any_of("/\0"), boost::token_compress_on);
        }
    } catch (std::exception& e) {
        logger::LogJsonAndMessage(json_helper::CreateErrorValue("", e.what()), "in GetVectorFromTarget");
    }
    logger::LogJsonAndMessage({}, "in GetVectorFromTarget after splitting");
    return result;
}

}