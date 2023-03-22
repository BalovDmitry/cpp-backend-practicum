cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB gameserver_SRC "src/*.h" "src/*.cpp")
add_executable(game_server ${gameserver_SRC})

target_link_libraries(game_server ${Boost_LIBRARIES} Threads::Threads CONAN_PKG::boost)
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
