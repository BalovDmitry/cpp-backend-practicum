cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CTest)
include(${CONAN_BUILD_DIRS_CATCH2}/Catch.cmake) 

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(
  model_lib STATIC
    src/boost_json.cpp    
    src/model_map.h
    src/model_map.cpp
    src/model_dog.h
    src/model_dog.cpp
    src/model_player.h
    src/game_session.h
    src/game_session.cpp
    src/player_tokens.h
    src/player_tokens.cpp
    src/extra_data.h
    src/extra_data.cpp
    src/logger.h
    src/logger.cpp
    src/model_utils.h
    src/model_utils.cpp
    src/json_helper.h
    src/json_helper.cpp
    src/loot_generator.h
    src/loot_generator.cpp
    src/game.h
    src/game.cpp
    src/game_server_exceptions.h
    src/tagged.h
    src/geom.h
    src/collision_detector.h
    src/collision_detector.cpp
)
target_include_directories(model_lib PUBLIC CONAN_PKG::boost)
target_link_libraries(model_lib PUBLIC ${Boost_LIBRARIES} Threads::Threads CONAN_PKG::boost)

add_executable(
  game_server
    src/command_line_parser.h
    src/command_line_parser.cpp
    src/http_server.h
    src/http_server.cpp
    src/json_loader.h
    src/json_loader.cpp
    src/main.cpp
    src/path_helper.h
    src/path_helper.cpp
    src/request_handler_helper.h
    src/request_handler_helper.cpp
    src/request_handler_strategy.h
    src/request_handler_strategy.cpp
    src/request_handler.h
    src/ticker.h
    src/ticker.cpp
)

add_executable(
  game_server_tests
    tests/model-tests.cpp
    tests/loot_generator_tests.cpp
    tests/collision-detector-tests.cpp
    tests/main.cpp
)

target_link_libraries(game_server model_lib)
target_link_libraries(game_server_tests model_lib CONAN_PKG::catch2) 
catch_discover_tests(game_server_tests) 