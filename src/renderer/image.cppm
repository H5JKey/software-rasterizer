module;

#include <algorithm>
#include <cmath>
#include <vector>

export module renderer.image;
import math;

export enum class Filter { Nearest, Bilinear };

export class Image {
   public:
    int width, height;
    std::vector<Color> pixels;

    Image(int width, int height) : width(width), height(height), pixels(width * height) {}

    Color& operator[](std::size_t idx) {return pixels[idx];}
    const Color& operator[](std::size_t idx) const {return pixels[idx];}

    void resize(int width, int height) {
        this->width = width;
        this->height = height;

        pixels.resize(width * height);
    }

    void upscaleFrom(const Image& other, Filter filter) {
        if (other.width == width && other.height == height) {
            std::copy(other.pixels.begin(), other.pixels.end(), pixels.begin());
            return;
        }
        const int srcWidth = other.width;
        const int srcHeight = other.height;
        if (srcWidth <= 0 || srcHeight <= 0 || width <= 0 || height <= 0) return;

        switch (filter) {
            case Filter::Nearest:
#pragma omp parallel for schedule(static) if (width * height >= 16384)
                for (int y = 0; y < height; y++) {
                    const float srcY = (y + 0.5f) * srcHeight / height - 0.5f;
                    const std::size_t row = static_cast<std::size_t>(y) * width;
                    for (int x = 0; x < width; x++) {
                        const std::size_t srcRow = static_cast<std::size_t>(srcY) * srcWidth;

                        const int srcX = static_cast<int>((x + 0.5f) * srcWidth / width);
                        pixels[row + x] = other.pixels[srcRow + srcX];
                    }
                }
                break;
            case Filter::Bilinear:
#pragma omp parallel for schedule(static) if (width * height >= 16384)
                for (int y = 0; y < height; y++) {
                    const float srcY = (y + 0.5f) * srcHeight / height - 0.5f;
                    const std::size_t row = static_cast<std::size_t>(y) * width;
                    for (int x = 0; x < width; x++) {
                        const float srcX = (x + 0.5f) * srcWidth / width - 0.5f;

                        int i = static_cast<int>(std::floor(srcX));
                        int j = static_cast<int>(std::floor(srcY));

                        float a = srcX - i;
                        float b = srcY - j;

                        std::size_t left = static_cast<std::size_t>(std::clamp(i, 0, srcWidth - 1));
                        std::size_t right = static_cast<std::size_t>(std::clamp(i + 1, 0, srcWidth - 1));
                        std::size_t up = static_cast<std::size_t>(std::clamp(j, 0, srcHeight - 1));
                        std::size_t down = static_cast<std::size_t>(std::clamp(j + 1, 0, srcHeight - 1));

                        Color color00 = other.pixels[up * srcWidth + left];
                        Color color10 = other.pixels[up * srcWidth + right];
                        Color color01 = other.pixels[down * srcWidth + left];
                        Color color11 = other.pixels[down * srcWidth + right];

                        pixels[row + x] = color00 * (1 - a) * (1 - b) + color10 * a * (1 - b) + color01 * (1 - a) * b +
                                          color11 * a * b;
                    }
                }
                break;
        }
    }

    void clear() { std::fill(pixels.begin(), pixels.end(), Color(0.0f, 0.0f, 0.0f, 1.0f)); }
};
