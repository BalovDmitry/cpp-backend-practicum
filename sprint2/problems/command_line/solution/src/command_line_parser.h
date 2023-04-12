#pragma once

#include <boost/program_options.hpp>
#include <string>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

namespace command_line {

struct Args {

    int tick_period = 0;
    bool randomize_spawn_point = false;
    fs::path config_file;
    fs::path source_dir;
};

std::optional<Args> ParseCommandLine(int argc, const char* const argv[]);

} // namespace command_line
