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

    Color sample(vec2 uv) const;

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
