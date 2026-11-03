module;
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <vector>

export module renderer.shaders;

import renderer.framebuffer;
import model.material;
import math;
import scene.scene;
import scene.skybox;

export namespace shaders {

/* Vertex shaders */

struct VertexShaderUniforms {
    const mat4* VP;
    const std::vector<mat4>* M;
    const std::vector<mat3>* inverseTransposeM;
};

struct VertexShaderInput {
    vec3 position;
    vec3 normal;
    std::size_t instance_id;
};

struct VertexShaderOutput {
    vec4 position;
    vec3 normal;
};

using VertexShader = std::function<VertexShaderOutput(const VertexShaderInput&, const VertexShaderUniforms&)>;

/* Fragment shaders */

struct FragmentShaderUniforms {
    const Scene::Lighting* lighting;
    const Material* material;
    float minDepth, maxDepth;
    bool hasTexcoords;
};

struct FragmentShaderInput {
    vec3 normal;
    vec2 uv;
    float depth;
};

struct FragmentShaderOutput {
    Color color;
};

using FragmentShader = std::function<FragmentShaderOutput(const FragmentShaderInput&, const FragmentShaderUniforms&)>;

struct SkyboxShaderUniforms {
    const Skybox* skybox;
    const mat4* inversedVP;
    const vec3* cameraPosition;
};

struct SkyboxShaderInput {
    vec2 screenPosition;
};

struct SkyboxShaderOutput {
    Color color;
};

using SkyboxShader = std::function<SkyboxShaderOutput(const SkyboxShaderInput&, const SkyboxShaderUniforms&)>;

inline const VertexShader vertexShader = [](const VertexShaderInput& input,
                                            const VertexShaderUniforms& uniforms) -> VertexShaderOutput {
    VertexShaderOutput output;
    const auto& VP = *uniforms.VP;
    const auto& M = *uniforms.M;
    const auto& inverseTransposeM = *uniforms.inverseTransposeM;
    output.position = VP * M[input.instance_id] * vec4(input.position.x, input.position.y, input.position.z, 1.0);
    output.normal = inverseTransposeM[input.instance_id] * input.normal;
    return output;
};

inline const FragmentShader lambertShader = [](const FragmentShaderInput& input,
                                               const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
    FragmentShaderOutput output;
    const auto& material = *uniforms.material;
    const auto& lighting = *uniforms.lighting;

    if (uniforms.hasTexcoords && material.texture) {
        output.color = material.texture->sample(input.uv);
    } else {
        output.color = material.baseColor;
    }
    Color factor =
        (lighting.ambientColor + lighting.light.color * std::max(0.f, dot(input.normal, lighting.light.direction)));
    output.color.r = output.color.r * factor.r;
    output.color.g = output.color.g * factor.g;
    output.color.b = output.color.b * factor.b;
    return output;
};

inline const FragmentShader unlitShader = [](const FragmentShaderInput& input,
                                             const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
    FragmentShaderOutput output;
    const auto& material = *uniforms.material;
    if (uniforms.hasTexcoords && material.texture) {
        output.color = material.texture->sample(input.uv);
    } else {
        output.color = material.baseColor;
    }
    return output;
};

inline const FragmentShader normalsShader = [](const FragmentShaderInput& input,
                                               const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
    FragmentShaderOutput output;
    output.color = Color((input.normal.x + 1.f) / 2.f, (input.normal.y + 1.f) / 2.f, (input.normal.z + 1.f) / 2.f, 1.f);
    return output;
};

inline const FragmentShader texcoordsShader = [](const FragmentShaderInput& input,
                                                 const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
    FragmentShaderOutput output;
    if (uniforms.hasTexcoords)
        output.color = Color(input.uv.x, input.uv.y, 0.0f, 1.0f);
    else
        output.color = Color(0, 0, 0, 1);
    return output;
};

inline const FragmentShader normalizeDepthColors = [](const FragmentShaderInput& input,
                                                      const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
    FragmentShaderOutput output;
    float gray = input.depth;
    if (uniforms.minDepth < uniforms.maxDepth && std::isfinite(gray)) {
        float denum = 1.0f / (uniforms.maxDepth - uniforms.minDepth);
        gray = (gray - uniforms.minDepth) * denum;
        output.color = Color(gray, gray, gray, 1.0f);
    } else {
        output.color = Color(0, 0, 0, 1);
    }
    return output;
};

inline const SkyboxShader skyboxShader = [](const SkyboxShaderInput& input,
                                            const SkyboxShaderUniforms& uniforms) -> SkyboxShaderOutput {
    SkyboxShaderOutput output;
    const auto& inversedVP = *uniforms.inversedVP;

    vec4 world = inversedVP * vec4(input.screenPosition.x, input.screenPosition.y, 1.0f, 1.0f);
    world *= 1.0f / world.w;

    const vec3 rayDir = vec3(world.x, world.y, world.z) - *uniforms.cameraPosition;
    output.color = (*uniforms.skybox).sample(rayDir);
    return output;
};
}  // namespace shaders