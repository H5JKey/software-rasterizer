#include "texture.hpp"

#include "image-utils.hpp"

Color Texture::sample(vec2 uv) const {
    float x = uv.x * width;
    float y = uv.y * height;
    int left = std::min(static_cast<int>(x), static_cast<int>(width) - 1);
    int right = std::min(static_cast<int>(x) + 1, static_cast<int>(width) - 1);

    int down = std::min(static_cast<int>(y) + 1, static_cast<int>(height) - 1);
    int up = std::min(static_cast<int>(y), static_cast<int>(height) - 1);

    Color leftUpColor = pixels[up * width + left];
    Color leftDownColor = pixels[down * width + left];
    Color rightUpColor = pixels[up * width + right];
    Color rightDownColor = pixels[down * width + right];

    float rightWeight = x - left;
    float leftWeight = 1.0f - rightWeight;

    float downWeight = y - up;
    float upWeight = 1.0f - downWeight;

    return (leftUpColor * leftWeight + rightUpColor * rightWeight) * upWeight +
           (leftDownColor * leftWeight + rightDownColor * rightWeight) * downWeight;
}

Texture Texture::loadFromFile(const std::filesystem::path& filename) {
    int width, height;
    std::vector<float> rgb_f32 = load_image_f32_rgb(filename, width, height, 2.2f);
    std::vector<Color> pixels(width * height);
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            std::size_t idx = row * width + col;
            pixels[idx] = Color(rgb_f32[3 * idx + 0], rgb_f32[3 * idx + 1], rgb_f32[3 * idx + 2], 1.0f);
        }
    }
    return Texture(width, height, std::move(pixels));
}