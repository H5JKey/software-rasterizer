#pragma once

#include <vector>

#include "vectors.hpp"

struct Mesh {
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    std::vector<int> indices;
};
