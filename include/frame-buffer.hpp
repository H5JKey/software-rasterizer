#pragma once
#include <vector>

#include "vectors.hpp"

class FrameBuffer {
   public:
    size_t width, height;
    std::vector<Color> pixels;

    FrameBuffer(size_t width, size_t height) : width(width), height(height), pixels(width * height) {}
};