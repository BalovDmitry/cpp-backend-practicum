#include "path_helper.h"

//#include <iomanip>
//#include <cctype>

namespace path_helper {

bool IsSubPath(fs::path path, fs::path base) {
    // Приводим оба пути к каноничному виду (без . и ..)
    path = fs::weakly_canonical(path);
    base = fs::weakly_canonical(base);

    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            
            return false;
        }
    }
    return true;
}

fs::path GetAbsPath(const fs::path& basePath, const fs::path& relPath) {
    if (relPath.empty() || relPath.string() == "/") {
        return fs::weakly_canonical(basePath / fs::path("index.html"));
    }
    return fs::weakly_canonical(basePath / relPath);
}

fs::path GetRelPathFromRequest(const std::vector<std::string>& splittedRequest) {
    fs::path result;

    if (splittedRequest.empty()) {
        result = "index.html";
    } else if (splittedRequest.size() == 1) {
        result = splittedRequest.back();
    } else if (splittedRequest.size() == 2) {
        result = splittedRequest.front() + "/" + splittedRequest.back();
    }

    return result;
}

fs::path GetDecodedPath(std::string_view path) {   
    auto decoded_path = UrlDecode(path);
    if (!decoded_path.empty()) {
        return fs::path(decoded_path.begin() + 1, decoded_path.end());
    }
    return decoded_path;
}

std::string UrlDecode(const std::string_view& value) {
    std::string ret;
    char ch;
    int temp;
    for (int i = 0; i < value.length(); ++i) {
        if (value[i] == '%') {
            sscanf(value.substr(i + 1, 2).data(), "%x", &temp);
            ch = static_cast<char>(temp);
            ret += ch;
            i += 2;
        } else {
            ret += value[i];
        }
    }
    return ret;
}

}