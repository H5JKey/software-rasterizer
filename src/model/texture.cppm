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
    struct MipLevel {
        std::vector<Color> pixels;
        std::size_t width, height;
        Color sample(vec2 uv) const;
    };

    void buildMipLevels(std::size_t width, std::size_t height, std::vector<Color> pixels);

    Texture(std::size_t width, std::size_t height, std::vector<Color> pixels) {
        buildMipLevels(width, height, std::move(pixels));
    }

   public:
    std::vector<MipLevel> mipLevels;

    Color sample(vec2 uv, float lod) const {
        lod = std::clamp(lod, 0.0f, static_cast<float>(mipLevels.size() - 1));
        auto l0 = static_cast<std::size_t>(std::floor(lod));
        auto l1 = std::min(l0 + 1, mipLevels.size() - 1);
        float t = lod - static_cast<float>(l0);
        return mipLevels[l0].sample(uv) * (1.0 - t) + mipLevels[l1].sample(uv) * t;
    }

    static Texture loadFromFile(const std::filesystem::path& filename);
};
