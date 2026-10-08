#pragma once
#include <array>

template <std::size_t N>
struct vec {
    std::array<float, N> data;
    constexpr float& operator[](std::size_t idx);
};

template <>
struct vec<2> {
    union {
        std::array<float, 2> data;
        struct {
            float x, y;
        };
    };
    vec(float x, float y) : x(x), y(y) {}
    vec() : x(0), y(0) {}
};

template <>
struct vec<3> {
    union {
        std::array<float, 3> data;
        struct {
            float x, y, z;
        };
    };
    vec(float x, float y, float z) : x(x), y(y), z(z) {}
    vec() : x(0), y(0), z(0) {}
};

template <>
struct vec<4> {
    union {
        std::array<float, 4> data;
        struct {
            float x, y, z, w;
        };
        struct {
            float r, g, b, a;
        };
    };
    vec(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    vec() : x(0), y(0), z(0), w(0) {}
};

using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;
using Color = vec<4>;

template <std::size_t N>
constexpr vec<N> min(vec<N> v1, vec<N> v2) noexcept {
    return vec<N>(std::min(v1.x, v2.x), std::min(v1.y, v2.y));
}

template <std::size_t N>
constexpr vec<N> max(vec<N> v1, vec<N> v2) noexcept {
    return vec<N>(std::max(v1.x, v2.x), std::max(v1.y, v2.y));
}

float edgeFunction(vec2 a, vec2 b, vec2 p);