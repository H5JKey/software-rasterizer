module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <numeric>
#include <random>
#include <vector>

export module utils.perlin;

import math;
import utils.images;

export class Perlin2D {
   public:
    Perlin2D(std::uint32_t seed, int octaves) : octaves(std::max(1, octaves)) {
        std::array<std::uint8_t, 256> p{};
        std::iota(p.begin(), p.end(), 0);

        std::mt19937 rng(seed);
        std::shuffle(p.begin(), p.end(), rng);

        for (std::size_t i = 0; i < p.size(); ++i) {
            perm[i] = p[i];
            perm[i + p.size()] = p[i];
        }
    }

    float sample(vec2 p) const {
        float result = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float amplitudeSum = 0.0f;

        for (int octaveIndex = 0; octaveIndex < octaves; ++octaveIndex) {
            result += amplitude * octave(p * frequency);
            amplitudeSum += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return result / amplitudeSum;
    }

    void saveToPng(std::size_t width, std::size_t height, const std::filesystem::path& filename) const {
        constexpr float scale = 0.05f;
        std::vector<float> pixels(width * height * 3);
        for (std::size_t y = 0; y < height; ++y) {
            for (std::size_t x = 0; x < width; ++x) {
                const float value = sample(vec2(static_cast<float>(x), static_cast<float>(y)) * scale) * 0.5f + 0.5f;
                const std::size_t i = (y * width + x) * 3;
                pixels[i + 0] = value;
                pixels[i + 1] = value;
                pixels[i + 2] = value;
            }
        }
        save_image_f32_png_rgb(pixels.data(), filename, width, height, 3, 1.0f);
    }

   private:
    float octave(vec2 p) const {
        static constexpr vec2 gradients[] = {
            vec2(1.0f, 0.0f),
            vec2(-1.0f, 0.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, -1.0f),
            vec2(0.70710678f, 0.70710678f),
            vec2(-0.70710678f, 0.70710678f),
            vec2(0.70710678f, -0.70710678f),
            vec2(-0.70710678f, -0.70710678f),
        };

        const int xi = static_cast<int>(std::floor(p.x));
        const int yi = static_cast<int>(std::floor(p.y));
        const float xf = p.x - static_cast<float>(xi);
        const float yf = p.y - static_cast<float>(yi);

        const float u = xf * xf * xf * (xf * (xf * 6.0f - 15.0f) + 10.0f);
        const float v = yf * yf * yf * (yf * (yf * 6.0f - 15.0f) + 10.0f);

        const int x0 = static_cast<int>(static_cast<std::uint32_t>(xi) & 255u);
        const int x1 = static_cast<int>(static_cast<std::uint32_t>(xi + 1) & 255u);
        const int y0 = static_cast<int>(static_cast<std::uint32_t>(yi) & 255u);
        const int y1 = static_cast<int>(static_cast<std::uint32_t>(yi + 1) & 255u);

        const vec2& g00 = gradients[perm[perm[x0] + y0] & 7];
        const vec2& g10 = gradients[perm[perm[x1] + y0] & 7];
        const vec2& g01 = gradients[perm[perm[x0] + y1] & 7];
        const vec2& g11 = gradients[perm[perm[x1] + y1] & 7];

        const float n00 = dot(g00, vec2(xf, yf));
        const float n10 = dot(g10, vec2(xf - 1.0f, yf));
        const float n01 = dot(g01, vec2(xf, yf - 1.0f));
        const float n11 = dot(g11, vec2(xf - 1.0f, yf - 1.0f));

        const float nx0 = n00 + (n10 - n00) * u;
        const float nx1 = n01 + (n11 - n01) * u;

        return (nx0 + (nx1 - nx0) * v) * 1.41421356f;
    }

    std::array<std::uint8_t, 512> perm{};
    int octaves;
};
