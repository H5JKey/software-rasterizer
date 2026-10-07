#ifndef VECTORS_H
#define VECTORS_H
#include <stdexcept>

struct vec2 {
    float x, y;

    vec2(float x, float y) : x(x), y(y) {}
    vec2() : x(0), y(0) {}

    float& operator[](int idx);
};

struct vec3 {
    float x, y, z;

    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3() : x(0), y(0), z(0) {}

    float& operator[](int idx);
};

#endif
