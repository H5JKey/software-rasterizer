#include <cstdlib>
#include <iostream>
#include <print>

#include "mesh.hpp"

int main(int argc, char** argv) try {
    if (argc != 2) {
        std::println("Usage: {} <mesh.obj> <output.png>", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
