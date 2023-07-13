#pragma once

#include <filesystem>
#include <cassert>
#include <vector>
#include <string>

using namespace std::literals;
namespace fs = std::filesystem;

namespace path_helper {

// Возвращает true, если каталог p содержится внутри base_path.
bool IsSubPath(fs::path path, fs::path base);

fs::path CreatePathForTemporaryFile(const fs::path& basePath);
fs::path GetAbsPath(const fs::path& basePath, const fs::path& relPath);
fs::path GetRelPathFromRequest(const std::vector<std::string>& splittedRequest);
fs::path GetDecodedPath(std::string_view path);
std::string UrlDecode(const std::string_view& value);

}