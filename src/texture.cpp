#include "texture.hpp"

#include "image-utils.hpp"

Color Texture::sample(vec2 uv) const {
    float x = uv.x * width - 0.5;
    float y = uv.y * height - 0.5;

    std::size_t i = floor(x);
    std::size_t j = floor(y);
    float a = x - i;
    float b = y - j;

    std::size_t left = i;
    std::size_t right = (i + 1) % width;
    std::size_t up = j;
    std::size_t down = (j + 1) % height;

    Color color00 = pixels[up * width + left];
    Color color10 = pixels[up * width + right];
    Color color01 = pixels[down * width + left];
    Color color11 = pixels[down * width + right];

    return color00 * (1 - a) * (1 - b) + color10 * a * (1 - b) + color01 * (1 - a) * b + color11 * a * b;
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