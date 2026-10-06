#ifndef MESH_UTILS_H
#define MESH_UTILS_H
#include <filesystem>
#include <vector>

#include "vectors.hpp"

struct Mesh {
    std::vector<vec3> verts;
    std::vector<vec3> normals;
    std::vector<vec2> tcs;
    std::vector<int> indices;
};

// При ошибке возвращает пустой меш
Mesh load_obj(std::filesystem::path filename);

#endif
