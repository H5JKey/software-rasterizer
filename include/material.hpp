#pragma once
#include "math/vectors.hpp"
#include "texture.hpp"

struct Material {
    Color baseColor = Color(1, 1, 1, 1);
    const Texture* texture = nullptr;
};