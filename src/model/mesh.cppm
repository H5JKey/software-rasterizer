module;
#include <filesystem>
#include <vector>

export module model.mesh;
import math;

export struct AABB {
    vec3 min;
    vec3 max;

    vec3 center() { return (min + max) * 0.5f; }
    vec3 size() { return max - min; }

    std::array<vec3, 8> corners() const {
        std::array<vec3, 8> res;
        int idx = 0;
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < 2; k++)
                    res[idx++] = vec3(i == 0 ? min.x : max.x, j == 0 ? min.y : max.y, k == 0 ? min.z : max.z);

        return res;
    }
};

export class Mesh {
    AABB aabb_;

   public:
    Mesh() = default;
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    std::vector<int> indices;

    static Mesh loadFromFile(const std::filesystem::path& filename);
    AABB aabb() const { return aabb_; }

   private:
    void buildNormals();
    AABB buildAABB() const;
    void normalize();
};
