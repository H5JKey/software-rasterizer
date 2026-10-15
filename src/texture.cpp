#include "texture.hpp"

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