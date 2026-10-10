#pragma once
#include <limits>
#include <vector>

#include "math/vectors.hpp"

class FrameBuffer {
   public:
    size_t width, height;
    std::vector<Color> pixels;
    std::vector<float> depth;
    float maxDepth, minDepth;

    FrameBuffer(size_t width, size_t height)
        : width(width),
          height(height),
          pixels(width * height),
          depth(width * height, std::numeric_limits<float>::infinity()),
          maxDepth(-1),
          minDepth(1) {}

    void clear() {
        std::fill(pixels.begin(), pixels.end(), Color(0.0f, 0.0f, 0.0f, 1.0f));
        std::fill(depth.begin(), depth.end(), std::numeric_limits<float>::infinity());
        maxDepth = -1;
        minDepth = 1;
    }
};