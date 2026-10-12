#pragma once
#include <cstddef>
#include <math/vectors.hpp>
#include <vector>

struct Texture {
    std::size_t width, height;
    std::vector<Color> pixels;
    Texture(std::size_t width, std::size_t height, std::vector<Color> pixels)
        : width(width), height(height), pixels(std::move(pixels)) {}
    Texture() = default;
    Color sample(vec2 uv) const;
};