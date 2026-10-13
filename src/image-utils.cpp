#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "image-utils.hpp"
#include "stb_image.h"
#include "stb_image_write.h"

static inline unsigned char tonemap(float x, float a_gammaInv) {
    const int colorLDR = (int)(powf(x, a_gammaInv) * 255.0f + 0.5f);
    if (colorLDR < 0)
        return 0;
    else if (colorLDR > 255)
        return 255;
    else
        return colorLDR;
}

void save_image_f32_png_rgb(const std::vector<uint8_t>& data, const std::filesystem::path& filename, int w, int h,
                            int channels, float gamma) {
    const float gammaInv = 1.0f / gamma;
    std::vector<uint8_t> data_rgba8(4 * w * h);

    if (channels == 0)
        throw std::invalid_argument("Invalid image: channels must be greater than 0");
    else if (channels == 1) {
        for (int i = 0; i < w * h; i++) {
            data_rgba8[4 * i + 0] = tonemap(data[channels * i + 0], gammaInv);
            data_rgba8[4 * i + 1] = 0;
            data_rgba8[4 * i + 2] = 0;
            data_rgba8[4 * i + 3] = 255;
        }
    } else if (channels == 2) {
        for (int i = 0; i < w * h; i++) {
            data_rgba8[4 * i + 0] = tonemap(data[channels * i + 0], gammaInv);
            data_rgba8[4 * i + 1] = tonemap(data[channels * i + 1], gammaInv);
            data_rgba8[4 * i + 2] = 0;
            data_rgba8[4 * i + 3] = 255;
        }
    } else if (channels >= 3) {
        for (int i = 0; i < w * h; i++) {
            data_rgba8[4 * i + 0] = tonemap(data[channels * i + 0], gammaInv);
            data_rgba8[4 * i + 1] = tonemap(data[channels * i + 1], gammaInv);
            data_rgba8[4 * i + 2] = tonemap(data[channels * i + 2], gammaInv);
            data_rgba8[4 * i + 3] = 255;
        }
    }

    // swap y
    for (int i = 0; i < h / 2; i++) {
        for (int j = 0; j < w; j++) {
            int i1 = i * w + j;
            int i2 = (h - 1 - i) * w + j;
            for (int ch = 0; ch < 4; ch++) {
                unsigned char tmp = data_rgba8[4 * i1 + ch];
                data_rgba8[4 * i1 + ch] = data_rgba8[4 * i2 + ch];
                data_rgba8[4 * i2 + ch] = tmp;
            }
        }
    }

    if (stbi_write_png(filename.c_str(), w, h, 4, data_rgba8.data(), 4 * w) < 0) {
        throw std::runtime_error(std::format("Failed to save image {}", filename.string()));
    }
}
std::vector<float> load_image_f32_rgb(const std::filesystem::path& filename, int& w, int& h, float gamma) {
    constexpr int kChannels = 3;

    int file_channels = 0;
    stbi_uc* raw_data = stbi_load(filename.string().c_str(), &w, &h, &file_channels, kChannels);

    if (raw_data == nullptr) {
        throw std::runtime_error(
            std::format("Failed to load image '{}': {}", filename.string(), stbi_failure_reason()));
    }

    // RAII guard so raw_data is freed on any exception path below.
    struct StbiDeleter {
        void operator()(stbi_uc* p) const noexcept { STBI_FREE(p); }
    };
    std::unique_ptr<stbi_uc, StbiDeleter> raw_guard(raw_data);

    if (w <= 0 || h <= 0) {
        throw std::runtime_error(std::format("Invalid image dimensions for '{}': {}x{}", filename.string(), w, h));
    }

    // stb_image loads top-down; flip vertically into the output.
    const std::size_t row_bytes = static_cast<std::size_t>(w) * kChannels;
    std::vector<float> data_f32(static_cast<std::size_t>(w) * h * kChannels);
    const float inv255 = 1.0f / 255.0f;

    for (int y = 0; y < h; ++y) {
        const stbi_uc* src = raw_data + static_cast<std::size_t>(h - 1 - y) * row_bytes;
        float* dst = data_f32.data() + static_cast<std::size_t>(y) * row_bytes;
        for (int x = 0; x < w * kChannels; ++x) {
            dst[x] = std::pow(src[x] * inv255, gamma);
        }
    }

    return data_f32;
}