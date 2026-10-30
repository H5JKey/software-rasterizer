module;
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>

export module math:matrices;
import :vectors;

export template <std::size_t N>
struct Matrix {
    static constexpr std::size_t dim = N;
    std::array<float, N * N> data;

    template <typename... Args>
        requires(sizeof...(Args) == N * N)
    constexpr Matrix(Args... args) : data{static_cast<float>(args)...} {}
    constexpr Matrix() : data{} {};

    constexpr float& operator[](std::size_t idx) { return data[idx]; }
    constexpr const float& operator[](std::size_t idx) const { return data[idx]; }
    constexpr float& operator[](std::size_t row, std::size_t col) { return data[N * row + col]; }
    constexpr const float& operator[](std::size_t row, std::size_t col) const { return data[N * row + col]; }

    static constexpr Matrix<N> identity() {
        Matrix<N> result;
        for (std::size_t i = 0; i < N; i++) {
            result[i, i] = 1;
        }
        return result;
    }

    constexpr Vec<N> operator*(const Vec<N>& vec) const {
        Vec<N> result;
        for (std::size_t row = 0; row < N; row++) {
            float sum = float();
            for (std::size_t col = 0; col < N; col++) {
                sum += data[N * row + col] * vec[col];
            }
            result.data[row] = sum;
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
                result.data[row * N + col] = sum;
            }
        }
        return result;
    }

    constexpr Matrix<N> transpose() const {
        Matrix<N> result;
        for (std::size_t row = 0; row < N; row++) {
            for (std::size_t col = 0; col < N; col++) {
                result.data[N * col + row] = data[N * row + col];
            }
        }
        return result;
    }

    constexpr Matrix<N> inverse() const {
        Matrix<N> matrix = *this;
        Matrix<N> result = Matrix<N>::identity();
        for (std::size_t col = 0; col < N; ++col) {
            std::size_t pivot = col;
            auto maxAbs = std::abs(matrix[col, col]);
            for (std::size_t row = col + 1; row < N; ++row) {
                auto v = std::abs(matrix[row, col]);
                if (v > maxAbs) {
                    maxAbs = v;
                    pivot = row;
                }
            }
            if (maxAbs < 1e-6) throw std::runtime_error("matrix is singular or near-singular");
            if (pivot != col) {
                for (std::size_t j = 0; j < N; ++j) {
                    std::swap(matrix[col, j], matrix[pivot, j]);
                    std::swap(result[col, j], result[pivot, j]);
                }
            }
            float diag = matrix[col, col];
            for (std::size_t j = 0; j < N; ++j) {
                matrix[col, j] /= diag;
                result[col, j] /= diag;
            }

            for (std::size_t row = 0; row < N; ++row) {
                if (row == col) continue;
                float factor = matrix[row, col];
                if (factor == 0.0f) continue;
                for (std::size_t j = 0; j < N; ++j) {
                    matrix[row, j] -= factor * matrix[col, j];
                    result[row, j] -= factor * result[col, j];
                }
            }
        }
        return result;
    }
    template <std::size_t M>
    constexpr Matrix<M> upperLeft() const {
        Matrix<M> result;
        for (std::size_t row = 0; row < M; row++) {
            for (std::size_t col = 0; col < M; col++) {
                result[row, col] = data[row * N + col];
            }
        }
        return result;
    }
};

export using mat2 = Matrix<2>;
export using mat3 = Matrix<3>;
export using mat4 = Matrix<4>;