#pragma once

#include <boost/program_options.hpp>
#include <string>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

namespace command_line
{

struct Args {

    double tick_period = 0;
    bool randomize_spawn_point = false;
    bool debug_mode = true;
    fs::path config_file;
    fs::path source_dir;
};

std::optional<Args> ParseCommandLine(int argc, const char* const argv[]);

} // namespace command_line
