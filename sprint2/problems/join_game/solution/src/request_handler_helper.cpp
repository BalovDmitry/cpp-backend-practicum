#include "request_handler_helper.h"

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

namespace http_handler {

bool IsApiRequest(const StringRequest &req)
{
    auto splittedRequest = GetVectorFromTarget(req.target());

    if (!splittedRequest.empty() && splittedRequest.front() == "api") {
        return true;
    }
    return false;
}

std::vector<std::string> GetVectorFromTarget(const std::string_view& target)
{
    std::vector<std::string> result;

    if (target.size() > 1) {
        boost::split(result, std::string_view(target.data() + 1, target.size() - 1), boost::is_any_of("/\0"), boost::token_compress_on);
    }

    return result;
}

}