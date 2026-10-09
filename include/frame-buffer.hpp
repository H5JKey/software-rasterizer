#pragma once
#include <vector>

#include "math/vectors.hpp"

class FrameBuffer {
   public:
    size_t width, height;
    std::vector<Color> pixels;

    FrameBuffer(size_t width, size_t height) : width(width), height(height), pixels(width * height) {}

    void clear() { std::fill(pixels.begin(), pixels.end(), Color(0.0f, 0.0f, 0.0f, 1.0f)); }
};