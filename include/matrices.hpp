#pragma once
#include <array>

template <std::size_t N>
struct mat {
    std::array<float, N * N> data;
    constexpr float& operator[](std::size_t idx);
    constexpr float& operator[](std::size_t row, std::size_t col);
};
