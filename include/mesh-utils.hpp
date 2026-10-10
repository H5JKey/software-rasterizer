#pragma once
#include <filesystem>

#include "mesh.hpp"

struct AABB {
    vec3 min;
    vec3 max;

    vec3 center() { return (min + max) * 0.5f; }
    vec3 size() { return max - min; }
};

AABB calculateABB(const Mesh& mesh);

Mesh load_obj(const std::filesystem::path& filename);

mat4 calculateNormalizationMatrix(const Mesh& mesh);
