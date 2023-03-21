#include "path_helper.h"

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
    fs::path result;
    result = fs::weakly_canonical(basePath / relPath);
    return result;
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

}