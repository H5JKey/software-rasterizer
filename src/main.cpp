#include <iostream>
#include <print>
import app;

int main() try {
    App app(800, 600, "Software Rasterizer");
    app.run();
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}