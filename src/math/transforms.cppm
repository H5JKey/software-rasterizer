module;
#include <cmath>
export module math:transforms;

import :vectors;
import :matrices;

export constexpr mat4 translation(float offsetX, float offsetY, float offsetZ) {
    return mat4(1, 0, 0, offsetX, 0, 1, 0, offsetY, 0, 0, 1, offsetZ, 0, 0, 0, 1);
}

export constexpr mat4 translation(const vec3& offset) { return translation(offset.x, offset.y, offset.z); }

export constexpr mat4 rotationX(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return mat4(1, 0, 0, 0, 0, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1);
}

export constexpr mat4 rotationY(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return mat4(c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1);
}

export constexpr mat4 rotationZ(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return mat4(c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

export constexpr mat4 scale(float xFactor, float yFactor, float zFactor) {
    return mat4(xFactor, 0, 0, 0, 0, yFactor, 0, 0, 0, 0, zFactor, 0, 0, 0, 0, 1);
}

export constexpr mat4 scale(const vec3& factors) { return scale(factors.x, factors.y, factors.z); }

export constexpr mat4 lookAt(const vec3& eye, const vec3& forward, const vec3& up) {
    vec3 zc = normalize(forward);
    vec3 xc = normalize(cross(up, zc));
    vec3 yc = cross(zc, xc);

    return mat4(xc.x, xc.y, xc.z, -dot(xc, eye), yc.x, yc.y, yc.z, -dot(yc, eye), zc.x, zc.y, zc.z, -dot(zc, eye), 0, 0,
                0, 1);
}

export constexpr mat4 perspective(float fov, float aspect, float zNear, float zFar) {
    float c = 1 / tan(fov / 2);

    return mat4(c / aspect, 0, 0, 0, 0, c, 0, 0, 0, 0, (zFar + zNear) / (zNear - zFar),
                2 * zFar * zNear / (zNear - zFar), 0, 0, -1, 0);
}

export constexpr mat4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
    return mat4(2 / (right - left), 0, 0, -(right + left) / (right - left), 0, 2 / (top - bottom), 0,
                -(top + bottom) / (top - bottom), 0, 0, -2 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear), 0, 0, 0,
                1);
}