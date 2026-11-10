module;
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <span>
#include <string_view>

export module renderer.shaders;

import renderer.framebuffer;
import model.material;
import math;
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

inline constexpr VertexShader defaultVertexShader = [](const VertexShaderInput& input, const FrameUniforms& frame,
                                                       const DrawUniforms& draw) -> VertexShaderOutput {
    const vec4 world = (*draw.M) * vec4(input.position.x, input.position.y, input.position.z, 1.0f);
    return {
        .clipPosition = (*frame.VP) * world,
        .worldPosition = vec3(world.x, world.y, world.z),
        .normal = (*draw.inverseTransposeM) * input.normal,
        .uv = input.uv,
    };
};

inline constexpr VertexShader terrainVertexShader = [](const VertexShaderInput& input, const FrameUniforms& frame,
                                                       const DrawUniforms& draw) -> VertexShaderOutput {
    const auto& terrain = *draw.terrain;

    vec4 world = (*draw.M) * vec4(input.position.x, 0.0, input.position.z, 1.0f);

    const float e = terrain.step * 0.5f;
    const float h = terrain.heightAt(world.x, world.z);
    world.y = h;

    const float hL = terrain.heightAt(world.x - e, world.z);
    const float hR = terrain.heightAt(world.x + e, world.z);
    const float hD = terrain.heightAt(world.x, world.z - e);
    const float hU = terrain.heightAt(world.x, world.z + e);

    const float dhdx = (hR - hL) / (2.0f * e);
    const float dhdz = (hU - hD) / (2.0f * e);

    const vec3 normal = normalize(vec3(-dhdx, 1.0f, -dhdz));

    const float inv = terrain.invUvPeriod;
    const float u = world.x * inv;
    const float v = world.z * inv;
    return {
        .clipPosition = (*frame.VP) * world,
        .worldPosition = vec3(world.x, world.y, world.z),
        .normal = normal,
        .uv = vec2(u - std::floor(u), v - std::floor(v)),
    };
};

struct ShaderContext {
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
    float depth;
    Color color = Color(0, 0, 0, 1);
};

using FragmentShader = void (*)(ShaderContext&, const FrameUniforms&, const DrawUniforms&);

inline constexpr FragmentShader lambertShader = [](ShaderContext& context, const FrameUniforms& frame,
                                                   const DrawUniforms& draw) {
    const auto& material = *draw.material;
    Color color = context.color;

    const auto& lighting = *frame.lighting;
    const Color factor =
        lighting.ambientColor + lighting.light.color * std::max(0.f, dot(context.normal, lighting.light.direction));
    color.r *= factor.r;
    color.g *= factor.g;
    color.b *= factor.b;
    context.color = color;
};

inline constexpr FragmentShader materialShader = [](ShaderContext& context, const FrameUniforms& frame,
                                                    const DrawUniforms& draw) {
    const auto& material = *draw.material;
    float distance =
        2.0f * frame.zNear * frame.zFar / (frame.zFar + frame.zNear - context.depth * (frame.zFar - frame.zNear));
    float lod = std::log2(std::max(distance, 1e-4f));
    context.color =
        (draw.hasTexcoords && material.texture) ? material.texture->sample(context.uv, lod) : material.baseColor;
};

inline constexpr FragmentShader mipLevelShader = [](ShaderContext& context, const FrameUniforms& frame,
                                                    const DrawUniforms& draw) {
    const auto& material = *draw.material;
    if (!draw.hasTexcoords || material.texture == nullptr) {
        context.color = Color(0, 0, 0, 1);
        return;
    }

    float distance =
        2.0f * frame.zNear * frame.zFar / (frame.zFar + frame.zNear - context.depth * (frame.zFar - frame.zNear));
    float lod = std::log2(std::max(distance, 1e-4f));

    std::size_t levels = material.texture->mipLevels.size();
    float maxLod = static_cast<float>(levels > 0 ? levels - 1 : 0);
    lod = std::clamp(std::floor(lod), 0.0f, maxLod);

    const float t = maxLod > 0.0f ? lod / maxLod : 0.0f;
    const float phase = t * 4.71238898f;
    context.color = Color(0.5f + 0.5f * std::cos(phase), 0.5f + 0.5f * std::cos(phase + 2.09439510f),
                          0.5f + 0.5f * std::cos(phase + 4.18879020f), 1.0f);
};

inline constexpr FragmentShader normalsShader = [](ShaderContext& context, const FrameUniforms&,
                                                   const DrawUniforms& draw) {
    context.color =
        Color((context.normal.x + 1.f) / 2.f, (context.normal.y + 1.f) / 2.f, (context.normal.z + 1.f) / 2.f, 1.f);
};

inline constexpr FragmentShader texcoordsShader = [](ShaderContext& context, const FrameUniforms&,
                                                     const DrawUniforms& draw) {
    context.color = draw.hasTexcoords ? Color(context.uv.x, context.uv.y, 0.0f, 1.0f) : Color(0, 0, 0, 1);
};

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

inline constexpr ScreenShader skyboxShader = [](const ScreenShaderInput& input, const FrameUniforms& frame) -> Color {
    if (std::isfinite(input.depth)) return input.color;

    const mat4& inverseVP = *frame.inverseVP;
    vec4 world = inverseVP * vec4(input.ndcX, input.ndcY, 1.0f, 1.0f);
    world *= 1.0f / world.w;

    const vec3 rayDirection = vec3(world.x, world.y, world.z) - *frame.cameraPosition;
    return frame.skybox->sample(rayDirection);
};

inline constexpr ScreenShader normalizeDepthShader = [](const ScreenShaderInput& input, const FrameUniforms&) -> Color {
    if (input.minDepth < input.maxDepth && std::isfinite(input.depth)) {
        const float gray = (input.depth - input.minDepth) / (input.maxDepth - input.minDepth);
        return Color(gray, gray, gray, 1.0f);
    }
    return Color(0, 0, 0, 1);
};

struct Pipeline {
    std::string_view name;
    std::span<const FragmentShader> fragmentShaders{};
    std::span<const ScreenShader> screenShaders{};
};

inline constexpr FragmentShader lambertStages[] = {materialShader, lambertShader};
inline constexpr FragmentShader unlitStages[] = {materialShader};
inline constexpr FragmentShader normalsStages[] = {normalsShader};
inline constexpr FragmentShader texcoordsStages[] = {texcoordsShader};
inline constexpr FragmentShader mipLevelStages[] = {mipLevelShader};

inline constexpr ScreenShader skyboxStages[] = {skyboxShader};
inline constexpr ScreenShader depthStages[] = {normalizeDepthShader};

inline constexpr Pipeline passes[] = {
    {.name = "Lambert", .fragmentShaders = lambertStages, .screenShaders = skyboxStages},
    {.name = "Unlit", .fragmentShaders = unlitStages},
    {.name = "Depth", .screenShaders = depthStages},
    {.name = "Normals", .fragmentShaders = normalsStages},
    {.name = "Texcoords", .fragmentShaders = texcoordsStages},
    {.name = "MipLevels", .fragmentShaders = mipLevelStages},
};

inline constexpr std::size_t passesCount = std::size(passes);

}  // namespace shaders
