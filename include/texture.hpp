#pragma once
#include <cstddef>
#include <filesystem>
#include <math/vectors.hpp>
#include <vector>

class Texture {
    Texture(std::size_t width, std::size_t height, std::vector<Color> pixels)
        : width(width), height(height), pixels(std::move(pixels)) {}

   public:
    std::size_t width, height;
    std::vector<Color> pixels;

    Color sample(vec2 uv) const;

    static Texture loadFromFile(const std::filesystem::path& filename);
};
