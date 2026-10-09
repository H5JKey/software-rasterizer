#pragma once

#include "vectors.hpp"

template <std::size_t N>
struct Matrix {
    static constexpr std::size_t dim = N;
    std::array<float, N * N> data;

    template <typename... Args>
        requires(sizeof...(Args) == N * N)
    constexpr Matrix(Args... args) : data{static_cast<float>(args)...} {}
    constexpr Matrix() = default;

    constexpr float& operator[](std::size_t idx) { return data[idx]; }
    constexpr const float& operator[](std::size_t idx) const { return data[idx]; }
    constexpr float& operator[](std::size_t row, std::size_t col) { return data[N * row + col]; }
    constexpr const float& operator[](std::size_t row, std::size_t col) const { return data[N * row + col]; }

    constexpr Vec<N> operator*(const Vec<N>& vec) const {
        Vec<N> result;
        for (std::size_t row = 0; row < N; row++) {
            float sum = float();
            for (std::size_t col = 0; col < N; col++) {
                sum += data[N * row + col] * vec[col];
            }
            result[row] = sum;
        }
        return result;
    }

    constexpr Matrix<N> operator*(const Matrix<N>& other) const {
        Matrix<N> result;
        for (std::size_t row = 0; row < N; row++) {
            for (std::size_t col = 0; col < N; col++) {
                float sum = 0;
                for (std::size_t i = 0; i < N; i++) {
                    sum += data[N * row + i] * other[N * i + col];
                }
                result[row, col] = sum;
            }
        }
        return result;
    }

    constexpr Vec<N> transpose() const {
        Vec<N> result;
        for (std::size_t row = 0; row < N; row++) {
            for (std::size_t col = 0; col < N; col++) {
                result[N * col + row] = data[N * row + col];
            }
        }
        return result;
    }
};

using mat2 = Matrix<2>;
using mat3 = Matrix<3>;
using mat4 = Matrix<4>;