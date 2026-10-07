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
    int channels = 0;
    stbi_uc* raw_data = stbi_load(filename.c_str(), &w, &h, &channels, 3);
    if (channels != 3 || raw_data == NULL || w <= 0 || h <= 0) {
        if (raw_data) STBI_FREE(raw_data);
        throw std::runtime_error(std::format("Failed to load mage {}", filename.string()));
    }

    // swap y
    for (int i = 0; i < h / 2; i++) {
        for (int j = 0; j < w; j++) {
            int i1 = i * w + j;
            int i2 = (h - 1 - i) * w + j;
            for (int ch = 0; ch < channels; ch++) {
                unsigned char tmp = raw_data[channels * i1 + ch];
                raw_data[channels * i1 + ch] = raw_data[channels * i2 + ch];
                raw_data[channels * i2 + ch] = tmp;
            }
        }
    }

    int sz = 3 * w * h;
    std::vector<float> data_f32(sz);
    for (int i = 0; i < sz; i++) data_f32[i] = powf(raw_data[i] / 255.0f, gamma);

    STBI_FREE(raw_data);

    return data_f32;
}