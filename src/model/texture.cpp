module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <vector>

module model.texture;

const Texture& Texture::loadFromFile(const std::filesystem::path& filename) {
    int width, height;
    std::vector<float> rgb_f32 = load_image_f32_rgb(filename, width, height, 2.2f);
    std::vector<Color> pixels(width * height);
#pragma omp parallel for if (static_cast<long long>(width) * height >= 4096)
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            std::size_t idx = row * width + col;
            pixels[idx] = Color(rgb_f32[3 * idx + 0], rgb_f32[3 * idx + 1], rgb_f32[3 * idx + 2], 1.0f);
        }
    }
    static std::deque<Texture> loadedTextures;
    loadedTextures.emplace_back(width, height, std::move(pixels));
    return loadedTextures.back();
}

const TextureWithMipLevels& TextureWithMipLevels::loadFromFile(const std::filesystem::path& filename) {
    static std::deque<TextureWithMipLevels> loadedTextures;
    loadedTextures.emplace_back(Texture::loadFromFile(filename));
    return loadedTextures.back();
}

void TextureWithMipLevels::buildMipLevels(Texture texture) {
    mipLevels.push_back(std::move(texture));
    std::size_t width = texture.width;
    std::size_t height = texture.height;
    while (width > 1 || height > 1) {
        const std::size_t nextWidth = std::max<std::size_t>(1, (width + 1) / 2);
        const std::size_t nextHeight = std::max<std::size_t>(1, (height + 1) / 2);

        const Texture& src = mipLevels.back();
        std::vector<Color> nextPixels(nextWidth * nextHeight);

#pragma omp parallel for if (nextWidth * nextHeight >= 4096)
        for (std::size_t y = 0; y < nextHeight; ++y) {
            const std::size_t y0 = std::min(2 * y, src.height - 1);
            const std::size_t y1 = std::min(2 * y + 1, src.height - 1);

            for (std::size_t x = 0; x < nextWidth; ++x) {
                const std::size_t x0 = std::min(2 * x, src.width - 1);
                const std::size_t x1 = std::min(2 * x + 1, src.width - 1);

                nextPixels[y * nextWidth + x] = (src.pixels[y0 * src.width + x0] + src.pixels[y0 * src.width + x1] +
                                                 src.pixels[y1 * src.width + x0] + src.pixels[y1 * src.width + x1]) *
                                                0.25f;
            }
        }

        width = nextWidth;
        height = nextHeight;
        mipLevels.emplace_back(width, height, std::move(nextPixels));
    }
}