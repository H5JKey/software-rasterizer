#include <iostream>
#include <print>
#include <string>
#include <filesystem>
import app;

int main(int argc, char* argv[]) try {
    if (argc != 5 && argc != 6) {
        std::println("Usage: {} <viewport width> <viewport height> <renderer width> <renderer height> [filename to save heightmap]", argv[0]);
        return EXIT_FAILURE;
    }
    std::optional<std::filesystem::path> pathToSaveHeightmap;
    if (argc == 6)
        pathToSaveHeightmap = argv[5];
    App app(std::stoi(argv[1]), std::stoi(argv[2]), std::stoi(argv[3]), std::stoi(argv[4]), "Software Rasterizer", pathToSaveHeightmap);
    app.run();
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}