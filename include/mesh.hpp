#pragma once

#include <vector>

#include "math/matrices.hpp"
#include "math/vectors.hpp"

struct Mesh {
    mat4 normalizationMatrix = mat4::identity();
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    std::vector<int> indices;
};