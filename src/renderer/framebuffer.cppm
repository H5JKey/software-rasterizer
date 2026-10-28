module;
#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

export module renderer.framebuffer;

import math.vectors;

export class Framebuffer {
   public:
    std::size_t width, height;
    std::vector<Color> pixels;
    std::vector<float> depth;

    Framebuffer(std::size_t width, std::size_t height)
        : width(width),
          height(height),
          pixels(width * height),
          depth(width * height, std::numeric_limits<float>::infinity()) {}

    void resize(std::size_t width, std::size_t height) {
        this->width = width;
        this->height = height;

        pixels.resize(width * height);
        depth.resize(width * height);
    }

    void clear() {
        std::fill(pixels.begin(), pixels.end(), Color(0.0f, 0.0f, 0.0f, 1.0f));

        std::fill(depth.begin(), depth.end(), std::numeric_limits<float>::infinity());
    }
};