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
        const auto wrapIndex = [](int value, int size) -> std::size_t {
            if (value < 0 || value >= size) {
                value %= size;
                if (value < 0) value += size;
            }
            return static_cast<std::size_t>(value);
        };
        int w = static_cast<int>(width);
        int h = static_cast<int>(height);

        float x = uv.x * static_cast<float>(w) - 0.5f;
        float y = uv.y * static_cast<float>(h) - 0.5f;

        int i = static_cast<int>(x);
        i -= (x < static_cast<float>(i)); 
        int j = static_cast<int>(y);
        j -= (y < static_cast<float>(j)); 
        float a = x - static_cast<float>(i);
        float b = y - static_cast<float>(j);

        std::size_t left = wrapIndex(i, w);
        std::size_t right = wrapIndex(i + 1, w);
        std::size_t up = wrapIndex(j, h);
        std::size_t down = wrapIndex(j + 1, h);

        Color color00 = pixels[up * width + left];
        Color color10 = pixels[up * width + right];
        Color color01 = pixels[down * width + left];
        Color color11 = pixels[down * width + right];

        return color00 * (1.0f - a) * (1.0f - b) + color10 * a * (1.0f - b) + color01 * (1.0f - a) * b +
               color11 * a * b;
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
        const auto l0 = static_cast<std::size_t>(std::floor(lod));
        const auto l1 = std::min(l0 + 1, mipLevels.size() - 1);
        const float t = lod - static_cast<float>(l0);
        if (l0 == l1 || t == 0.0f) return mipLevels[l0].sample(uv);

        return mipLevels[l0].sample(uv) * (1.0f - t) + mipLevels[l1].sample(uv) * t;
    }
};
