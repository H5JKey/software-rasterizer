#ifndef VECTORS_H
#define VECTORS_H
#include <stdexcept>

struct vec2 {
    float x, y;

    vec2(float x, float y) : x(x), y(y) {}
    vec2() = default;

    float operator[](int idx) {
        switch (idx) {
            case 0:
                return x;
            case 1:
                return y;
            default:
                throw std::invalid_argument("vec2 index out of range");
        }
    }
};

struct vec3 {
    float x, y, z;

    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3() = default;

    float operator[](int idx) {
        switch (idx) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                throw std::invalid_argument("vec2 index out of range");
        }
    }
};

#endif
