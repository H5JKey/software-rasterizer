module;
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <utility>
#include <vector>

export module model.texture;
import utils.images;
import math;

export class Texture {
   public:
    Texture(std::size_t width, std::size_t height, std::vector<Color> pixels)
        : width(width), height(height), pixels(std::move(pixels)) {}

    std::size_t width, height;
    std::vector<Color> pixels;

    Color sample(vec2 uv) const {
        float x = uv.x * width - 0.5;
        float y = uv.y * height - 0.5;

        int i = static_cast<int>(std::floor(x));
        int j = static_cast<int>(std::floor(y));
        float a = x - i;
        float b = y - j;

        std::size_t left = (i + static_cast<int>(width)) % width;
        std::size_t right = (i + 1) % width;
        std::size_t up = (j + static_cast<int>(height)) % height;
        std::size_t down = (j + 1) % height;

        Color color00 = pixels[up * width + left];
        Color color10 = pixels[up * width + right];
        Color color01 = pixels[down * width + left];
        Color color11 = pixels[down * width + right];

        return color00 * (1 - a) * (1 - b) + color10 * a * (1 - b) + color01 * (1 - a) * b + color11 * a * b;
    }

    static const Texture& loadFromFile(const std::filesystem::path& filename);
};

export class TextureWithMipLevels {
    void buildMipLevels(Texture);

   public:
    std::vector<Texture> mipLevels;

    TextureWithMipLevels(Texture texture) { buildMipLevels(std::move(texture)); }

    static const TextureWithMipLevels& loadFromFile(const std::filesystem::path& filename);

    Color sample(vec2 uv, float lod) const {
        lod = std::clamp(lod, 0.0f, static_cast<float>(mipLevels.size() - 1));
        auto l0 = static_cast<std::size_t>(std::floor(lod));
        auto l1 = std::min(l0 + 1, mipLevels.size() - 1);
        float t = lod - static_cast<float>(l0);
        return mipLevels[l0].sample(uv) * (1.0 - t) + mipLevels[l1].sample(uv) * t;
    }
};
