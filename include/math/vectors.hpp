#pragma once
#include <sys/types.h>

#include <array>
#include <cmath>
#include <type_traits>

template <std::size_t N>
struct VecStorage {
    static constexpr std::size_t dim = N;
    std::array<float, N> data;

    template <typename... Args>
        requires(sizeof...(Args) == N)
    VecStorage(Args... args) : data{static_cast<float>(args)...} {}
};

template <>
struct VecStorage<2> {
    static constexpr std::size_t dim = 2;
    union {
        std::array<float, 2> data;
        struct {
            float x, y;
        };
    };
    constexpr VecStorage(float x, float y) : x(x), y(y) {}
    constexpr VecStorage() : x(0), y(0) {}
};

template <>
struct VecStorage<3> {
    static constexpr std::size_t dim = 3;
    union {
        std::array<float, 3> data;
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
    };
    constexpr VecStorage(float x, float y, float z) : x(x), y(y), z(z) {}
    constexpr VecStorage() : x(0), y(0), z(0) {}
};

template <>
struct VecStorage<4> {
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
    constexpr VecStorage(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    constexpr VecStorage() : x(0), y(0), z(0), w(0) {}
};

template <std::size_t N>
struct Vec : VecStorage<N> {
    constexpr Vec() : VecStorage<N>() {}

    template <typename... Args>
        requires(sizeof...(Args) == N)
    constexpr Vec(Args... args) : VecStorage<N>(static_cast<float>(args)...) {}

    constexpr float& operator[](std::size_t idx) { return this->data[idx]; }
    constexpr const float& operator[](std::size_t idx) const { return this->data[idx]; }

    constexpr Vec<N> operator+(const Vec& other) const {
        Vec<N> result;
        for (std::size_t idx = 0; idx < N; idx++) {
            result[idx] = this->data[idx] + other[idx];
        }
        return result;
    }

    constexpr Vec<N> operator-(const Vec& other) const {
        Vec<N> result;
        for (std::size_t idx = 0; idx < N; idx++) {
            result[idx] = this->data[idx] - other[idx];
        }
        return result;
    }

    template <typename S>
        requires(std::is_arithmetic_v<S>)
    constexpr Vec<N> operator*(S scalar) const {
        Vec<N> result;
        for (std::size_t idx = 0; idx < N; idx++) {
            result[idx] = this->data[idx] * scalar;
        }
        return result;
    }
};

template <std::size_t N>
constexpr Vec<N> min(const Vec<N>& v1, const Vec<N>& v2) {
    Vec<N> result;
    for (std::size_t idx = 0; idx < N; idx++) {
        result[idx] = std::min(v1[idx], v2[idx]);
    }
    return result;
}

template <std::size_t N>
constexpr Vec<N> max(const Vec<N>& v1, const Vec<N>& v2) {
    Vec<N> result;
    for (std::size_t idx = 0; idx < N; idx++) {
        result[idx] = std::max(v1[idx], v2[idx]);
    }
    return result;
}

template <std::size_t N>
constexpr float length(const Vec<N>& v) {
    float sum = 0;
    for (float val : v.data) sum += val * val;
    return sqrtf(sum);
}

template <std::size_t N>
constexpr Vec<N> normalize(const Vec<N>& v) {
    Vec<N> result;
    float len = length(v);
    for (std::size_t idx = 0; idx < N; idx++) result[idx] = v.data[idx] / len;
    return result;
}

template <std::size_t N>
constexpr float dot(const Vec<N>& a, const Vec<N>& b) {
    float result = 0;
    for (std::size_t idx = 0; idx < N; idx++) result += a[idx] * b[idx];
    return result;
}

using vec2 = Vec<2>;
using vec3 = Vec<3>;
using vec4 = Vec<4>;
using Color = Vec<4>;

constexpr float edgeFunction(const vec2& a, const vec2& b, const vec2& p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

constexpr vec3 cross(const vec3& a, const vec3& b) {
    return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}