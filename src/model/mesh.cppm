module;
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <vector>

export module model.mesh;
import math.vectors;
import math.transforms;

export struct AABB {
    vec3 min;
    vec3 max;

    vec3 center() { return (min + max) * 0.5f; }
    vec3 size() { return max - min; }
};

export class Mesh {
    Mesh() = default;

   public:
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    std::vector<int> indices;

    static Mesh loadFromFile(const std::filesystem::path& filename);

   private:
    void buildNormals();
    AABB buildAABB() const;
    void normalize();
};
