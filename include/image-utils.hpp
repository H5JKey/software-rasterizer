#pragma once

#include <filesystem>
#include <vector>

void save_image_f32_png_rgb(const float* data, const std::filesystem::path& filename, int w, int h, int channels,
                            float gamma);

std::vector<float> load_image_f32_rgb(const std::filesystem::path& filename, int& w, int& h, float gamma);
