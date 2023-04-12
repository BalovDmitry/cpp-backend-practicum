#include "command_line_parser.h"

#include <iostream>
#include <stdexcept>

using namespace std::literals;

namespace command_line {

std::optional<Args> ParseCommandLine(int argc, const char *const argv[])
{
    Args args;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options"s);
    desc.add_options()
        ("help,h", "produce help message")
        ("tick-period,t", po::value(&args.tick_period)->value_name("milliseconds"s), "set tick period")
        ("config-file,c", po::value(&args.config_file)->value_name("file"s), "set config file path")
        ("www-root,w", po::value(&args.source_dir)->value_name("dir"s), "set static files root")
        ("randomize-spawn-points", po::bool_switch(&args.randomize_spawn_point), "spawn dogs at random positions");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        std::cout << desc;
        return std::nullopt;
    }

    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("Config file isn't set!");
    }

    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("Static files directory isn't set!");
    }

    return args;
}

}