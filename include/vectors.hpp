#pragma once
#include <array>

template <typename T, std::size_t N>
struct VecStorage {
    static constexpr std::size_t dim = N;
    std::array<T, N> data;
};

template <typename T>
struct VecStorage<T, 2> {
    static constexpr std::size_t dim = 2;
    union {
        std::array<float, 2> data;
        struct {
            float x, y;
        };
    };
    constexpr VecStorage(T x, T y) : x(x), y(y) {}
    constexpr VecStorage() : x(0), y(0) {}
};

template <typename T>
struct VecStorage<T, 3> {
    static constexpr std::size_t dim = 3;
    union {
        std::array<float, 3> data;
        struct {
            float x, y, z;
        };
        struct {
            T r, g, b;
        };
    };
    constexpr VecStorage(T x, T y, T z) : x(x), y(y), z(z) {}
    constexpr VecStorage() : x(0), y(0), z(0) {}
};

template <typename T>
struct VecStorage<T, 4> {
    static constexpr std::size_t dim = 4;
    union {
        std::array<float, 4> data;
        struct {
            float x, y, z, w;
        };
        struct {
            float r, g, b, a;
        };
    };
    constexpr VecStorage(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    constexpr VecStorage() : x(0), y(0), z(0), w(0) {}
};

template <typename T, std::size_t N>
struct Vec : VecStorage<T, N> {
    constexpr Vec() : VecStorage<T, N>() {}

    template <typename... Args>
        requires(sizeof...(Args) == N)
    constexpr Vec(Args... args) : VecStorage<T, N>(static_cast<T>(args)...) {}

    constexpr T& operator[](std::size_t idx) { return this->data[idx]; }
    constexpr const T& operator[](std::size_t idx) const { return this->data[idx]; }

    constexpr Vec<T, N> operator+(const Vec& other) {
        Vec<T, N> result;
        for (std::size_t idx = 0; idx < N; idx++) {
            result[idx] = this->data[idx] + other[idx];
        }
        return result;
    }

    constexpr Vec<T, N> operator*(T scalar) {
        Vec<T, N> result;
        for (std::size_t idx = 0; idx < N; idx++) {
            result[idx] = this->data[idx] * scalar;
        }
        return result;
    }
};

template <typename T, std::size_t N>
constexpr Vec<T, N> min(const Vec<T, N>& v1, const Vec<T, N>& v2) {
    Vec<T, N> result;
    for (std::size_t idx = 0; idx < N; idx++) {
        result[idx] = std::min(v1[idx], v2[idx]);
    }
    return result;
}

template <typename T, std::size_t N>
constexpr Vec<T, N> max(const Vec<T, N>& v1, const Vec<T, N>& v2) {
    Vec<T, N> result;
    for (std::size_t idx = 0; idx < N; idx++) {
        result[idx] = std::max(v1[idx], v2[idx]);
    }
    return result;
}

using vec2 = Vec<float, 2>;
using vec3 = Vec<float, 3>;
using vec4 = Vec<float, 4>;
using Color = Vec<float, 4>;

float edgeFunction(vec2 a, vec2 b, vec2 p);
