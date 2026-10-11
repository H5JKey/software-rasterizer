#pragma once
#include <limits>
#include <vector>

#include "math/vectors.hpp"

class FrameBuffer {
   public:
    size_t width, height;
    std::vector<Color> pixels;
    std::vector<float> depth;
    std::vector<Color> normals;

    FrameBuffer(size_t width, size_t height)
        : width(width),
          height(height),
          pixels(width * height),
          depth(width * height, std::numeric_limits<float>::infinity()),
          normals(width * height) {}

    void clear() {
        std::fill(pixels.begin(), pixels.end(), Color(0.0f, 0.0f, 0.0f, 1.0f));
        std::fill(normals.begin(), normals.end(), Color(0.0f, 0.0f, 0.0f, 1.0f));
        std::fill(depth.begin(), depth.end(), std::numeric_limits<float>::infinity());
    }
};