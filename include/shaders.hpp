#pragma once
#include <cmath>
#include <functional>

#include "frame-buffer.hpp"
#include "material.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "scene.hpp"
/* Vertex shaders */

struct VertexShaderUniforms {
    const mat4& VP;
    const std::vector<mat4>& M;
    const std::vector<mat3>& inverseTransposeM;
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
    const Scene::Lighting& lighting;
    const Material& material;
};

struct FragmentShaderInput {
    vec3 normal;
    vec2 uv;
    float depth;
    bool hasTexcoords;
};

struct FragmentShaderOutput {
    Color color;
};

using FragmentShader = std::function<FragmentShaderOutput(const FragmentShaderInput&, const FragmentShaderUniforms&)>;

/* Post processing shaders */

struct PostProcessingShaderUniforms {
    const FrameBuffer& frameBuffer;
    float minDepth, maxDepth;
};

struct PostProcessingShaderInput {
    std::size_t pixel_idx;
};

struct PostProcessingShaderOutput {
    Color color;
};

using PostProcessingShader =
    std::function<PostProcessingShaderOutput(const PostProcessingShaderInput&, const PostProcessingShaderUniforms&)>;

/* Default shaders*/

namespace defaultShaders {

inline const VertexShader vertexShader = [](const VertexShaderInput& input,
                                            const VertexShaderUniforms& uniforms) -> VertexShaderOutput {
    VertexShaderOutput output;
    output.position =
        uniforms.VP * uniforms.M[input.instance_id] * vec4(input.position.x, input.position.y, input.position.z, 1.0);
    output.normal = uniforms.inverseTransposeM[input.instance_id] * input.normal;
    return output;
};

inline const FragmentShader lambertShader = [](const FragmentShaderInput& input,
                                               const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
    FragmentShaderOutput output;
    if (input.hasTexcoords && uniforms.material.texture) {
        output.color = uniforms.material.texture->sample(input.uv);
    } else {
        output.color = uniforms.material.baseColor;
    }
    Color factor =
        (uniforms.lighting.ambientColor +
         uniforms.lighting.light.color * std::max(0.f, dot(input.normal, uniforms.lighting.light.direction)));
    output.color.r = output.color.r * factor.r;
    output.color.g = output.color.g * factor.g;
    output.color.b = output.color.b * factor.b;
    return output;
};

inline const FragmentShader unlitShader = [](const FragmentShaderInput& input,
                                             const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
    FragmentShaderOutput output;
    if (input.hasTexcoords && uniforms.material.texture) {
        output.color = uniforms.material.texture->sample(input.uv);
    } else {
        output.color = uniforms.material.baseColor;
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
    if (input.hasTexcoords)
        output.color = Color(input.uv.x, input.uv.y, 0.0f, 1.0f);
    else
        output.color = Color(0, 0, 0, 1);
    return output;
};

inline const PostProcessingShader normalizeDepthColors =
    [](const PostProcessingShaderInput& input,
       const PostProcessingShaderUniforms& uniforms) -> PostProcessingShaderOutput {
    PostProcessingShaderOutput output;
    float gray = uniforms.frameBuffer.depth[input.pixel_idx];
    if (uniforms.minDepth < uniforms.maxDepth && std::isfinite(gray)) {
        float denum = 1.0f / (uniforms.maxDepth - uniforms.minDepth);
        gray = (gray - uniforms.minDepth) * denum;
        output.color = Color(gray, gray, gray, 1.0f);
    } else {
        output.color = Color(0, 0, 0, 1);
    }
    return output;
};

}  // namespace defaultShaders
