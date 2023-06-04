#include <catch2/catch_session.hpp>
#include <iostream>

#include "../src/extra_data.h"

int main(int argc, char* argv[]) {
    int result = Catch::Session().run(argc, argv);
    return result;
} 