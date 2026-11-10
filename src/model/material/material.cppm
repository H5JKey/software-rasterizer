module;
#include <cstddef>
#include <vector>

export module model.material;

import math;
import model.texture;

export struct Material;

export namespace shaders {

struct MaterialInput {
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
    float depth = 0.0f;
    float lod = 0.0f;
    float time = 0.0f;
    bool hasTexcoords = false;
    std::size_t instanceId = 0;
    Color color = Color(0, 0, 0, 1);
    Color ambientColor = Color(0, 0, 0, 1);
    Color lightColor = Color(0, 0, 0, 1);
    vec3 lightDirection;
};

using MaterialShader = void (*)(MaterialInput&, const Material&);

};  // namespace shaders

export struct Material {
    Color baseColor = Color(1, 1, 1, 1);
    std::vector<const TextureWithMipLevels*> textures;
    shaders::MaterialShader shader = nullptr;
};
