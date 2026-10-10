#pragma once
#include "matrices.hpp"
#include "vectors.hpp"
#include <cmath>

constexpr mat4 translation(const vec3& offset) {
     return mat4(
        1, 0, 0, offset.x,
        0, 1, 0, offset.y,
        0, 0, 1, offset.z,
        0, 0, 0,     1
    );
}

constexpr mat4 rotationX(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return mat4(
        1, 0, 0, 0,
        0, c,-s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    );
}

constexpr mat4 rotationY(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return mat4(
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1
    );
}

constexpr mat4 rotationZ(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return mat4(
         c, -s, 0, 0,
         s,  c, 0, 0,
         0,  0, 1, 0,
         0,  0, 0, 1
    );
}

constexpr mat4 scale(const vec3& factors) {
    return mat4(
        factors.x, 0,         0,         0,
        0,         factors.y, 0,         0,
        0,         0,         factors.z, 0,
        0,         0,         0,         1
    );
}


constexpr mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up) {
    vec3 zc = normalize(eye - target);
    vec3 xc = normalize(cross(up, zc));
    vec3 yc = cross(zc, xc);

    return mat4(
        xc.x, xc.y, xc.z, -dot(xc, eye), 
        yc.x, yc.y, yc.z, -dot(yc, eye), 
        zc.x, zc.y, zc.z, -dot(zc, eye), 
        0,    0,    0,     1
    );
}

constexpr mat4 perspective(float fov, float aspect, float zNear, float zFar) {
    float c = 1 / tan(fov / 2);

    return mat4(
        c/aspect,   0,             0,                          0,
        0,          c,             0,                          0,
        0,          0,  (zFar+zNear) / (zNear - zFar), 2*zFar*zNear / (zNear - zFar),
        0,          0,            -1,                          0
    );
}