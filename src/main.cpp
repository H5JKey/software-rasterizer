#include <iostream>
#include <print>
import app;

int main(int argc, char* argv[]) try {
    if (argc != 5) {
        std::println("Usage: {} <viewport width> <viewport height> <renderer width> <renderer height>", argv[0]);
        return EXIT_FAILURE;
    }
    App app(std::stoi(argv[1]), std::stoi(argv[2]), std::stoi(argv[3]), std::stoi(argv[4]), "Software Rasterizer");
    app.run();
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}