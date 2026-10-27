module;
#include <limits>
#include <vector>

export module core.frameBuffer;

import math.vectors;

export class FrameBuffer {
   public:
    size_t width, height;
    std::vector<Color> pixels;
    std::vector<float> depth;

    FrameBuffer(size_t width, size_t height)
        : width(width),
          height(height),
          pixels(width * height),
          depth(width * height, std::numeric_limits<float>::infinity()) {}

    void clear() {
        std::fill(pixels.begin(), pixels.end(), Color(0.0f, 0.0f, 0.0f, 1.0f));

        std::fill(depth.begin(), depth.end(), std::numeric_limits<float>::infinity());
    }
};