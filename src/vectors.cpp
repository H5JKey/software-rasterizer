#include "vectors.hpp"

#include <stdexcept>

float& vec2::operator[](int idx) {
    switch (idx) {
        case 0:
            return x;
        case 1:
            return y;
        default:
            throw std::out_of_range("vec2 index out of range");
    }
}

float& vec3::operator[](int idx) {
    switch (idx) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            throw std::out_of_range("vec3 index out of range");
    }
}