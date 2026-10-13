#include "texture.hpp"

Color Texture::sample(vec2 uv) const {
    uv.u -= std::floor(uv.u);
    uv.v -= std::floor(uv.v);

    const int x = std::min(static_cast<int>(uv.u * width), static_cast<int>(width) - 1);
    const int y = std::min(static_cast<int>(uv.v * height), static_cast<int>(height) - 1);

    return pixels[static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)];
}