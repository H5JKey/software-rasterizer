#pragma once
#include "math/vectors.hpp"
#include "texture.hpp"

struct Material {
    const Texture* texture = nullptr;
    Color baseColor = Color(1, 1, 1, 1);
};