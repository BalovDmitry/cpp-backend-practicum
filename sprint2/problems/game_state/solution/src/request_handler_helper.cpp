#include "request_handler_helper.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

#include "logger.h"
#include "json_helper.h"

namespace http_handler {

bool IsApiRequest(const StringRequest &req)
{
    auto splittedRequest = GetVectorFromTarget(std::string_view(req.target().data(), req.target().size()));
    if (!splittedRequest.empty() && splittedRequest.front() == "api") {
        return true;
    }
    return false;
}

std::vector<std::string> GetVectorFromTarget(std::string_view target)
{
    std::vector<std::string> result;
    if (target.size() > 1) {
        boost::split(result, std::string_view(target.data() + 1, target.size() - 1), boost::is_any_of("/\0"), boost::token_compress_on);
    }
    return result;
}

}