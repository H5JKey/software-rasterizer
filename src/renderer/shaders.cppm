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

/* Vertex shaders */

export struct VertexShaderUniforms {
    const mat4& VP;
    const std::vector<mat4>& M;
    const std::vector<mat3>& inverseTransposeM;
};

export struct VertexShaderInput {
    vec3 position;
    vec3 normal;
    std::size_t instance_id;
};

export struct VertexShaderOutput {
    vec4 position;
    vec3 normal;
};

export using VertexShader = std::function<VertexShaderOutput(const VertexShaderInput&, const VertexShaderUniforms&)>;

/* Fragment shaders */

export struct FragmentShaderUniforms {
    const Scene::Lighting& lighting;
    const Material& material;
};

export struct FragmentShaderInput {
    vec3 normal;
    vec2 uv;
    float depth;
    bool hasTexcoords;
};

export struct FragmentShaderOutput {
    Color color;
};

export using FragmentShader =
    std::function<FragmentShaderOutput(const FragmentShaderInput&, const FragmentShaderUniforms&)>;

/* Post processing shaders */

export struct PostProcessingShaderUniforms {
    const Framebuffer& framebuffer;
    float minDepth, maxDepth;
};

export struct PostProcessingShaderInput {
    std::size_t pixel_idx;
};

export struct PostProcessingShaderOutput {
    Color color;
};

export using PostProcessingShader =
    std::function<PostProcessingShaderOutput(const PostProcessingShaderInput&, const PostProcessingShaderUniforms&)>;

/* Default shaders*/

export namespace defaultShaders {

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
    float gray = uniforms.framebuffer.depth[input.pixel_idx];
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
