#include "request_handler_helper.h"

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

#include "logger.h"

namespace http_handler {

bool IsApiRequest(const StringRequest &req)
{
    logger::LogJsonAndMessage({}, "in IsApiRequest");
    auto splittedRequest = GetVectorFromTarget(std::string_view(req.target().data(), req.target().size()));
    logger::LogJsonAndMessage({}, "after get splitted request");

    if (!splittedRequest.empty() && splittedRequest.front() == "api") {
        return true;
    }
    return false;
}

std::vector<std::string> GetVectorFromTarget(const std::string_view& target)
{
    std::vector<std::string> result;
    logger::LogJsonAndMessage({}, "in GetVectorFromTarget");
    if (target.size() > 1) {
        boost::split(result, std::string_view(target.data() + 1, target.size() - 1), boost::is_any_of("/\0"), boost::token_compress_on);
    }
    logger::LogJsonAndMessage({}, "in GetVectorFromTarget after splitting");
    return result;
}

}