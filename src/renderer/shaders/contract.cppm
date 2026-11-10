module;
#include <cstddef>

export module renderer.shaders:contract;

import math;
import model.material;
import scene.scene;
import scene.skybox;
import scene.terrain;

export namespace shaders {

struct FrameUniforms {
    const mat4* VP = nullptr;
    const mat4* inverseVP = nullptr;
    const vec3* cameraPosition = nullptr;
    const Scene::Lighting* lighting = nullptr;
    const Skybox* skybox = nullptr;
    float zNear = 0.0f;
    float zFar = 1.0f;
    float time = 0.0f;
};

struct DrawUniforms {
    const mat4* M = nullptr;
    const mat3* inverseTransposeM = nullptr;
    const Material* material = nullptr;
    std::size_t instanceId = 0;
    bool hasTexcoords = false;
    const Terrain* terrain = nullptr;
};

struct VertexShaderInput {
    vec3 position;
    vec3 normal;
    vec2 uv;
};

struct VertexShaderOutput {
    vec4 clipPosition;
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
};

using VertexShader = VertexShaderOutput (*)(const VertexShaderInput&, const FrameUniforms&, const DrawUniforms&);

struct ShaderContext {
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
    float depth;
    Color color = Color(0, 0, 0, 1);
};

using FragmentShader = void (*)(ShaderContext&, const FrameUniforms&, const DrawUniforms&);

struct ScreenShaderInput {
    int x;
    int y;
    float ndcX;
    float ndcY;
    float depth;
    Color color;
    float minDepth;
    float maxDepth;
};

using ScreenShader = Color (*)(const ScreenShaderInput&, const FrameUniforms&);

}  // namespace shaders
