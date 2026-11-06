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

export namespace shaders {

/* Uniforms */

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
};

/* Vertex stage */

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

/* Fragment stages
 *
 * Стадии выполняются по порядку и накапливают результат в context.color.
 * Поэтому пайплайн может состоять из нескольких шейдеров: например,
 * base shading, затем туман, затем гамма-коррекция.
 */

struct ShaderContext {
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
    float depth;
    std::size_t triangleId;
    Color color = Color(0, 0, 0, 1);
};

using FragmentShader = void (*)(ShaderContext&, const FrameUniforms&, const DrawUniforms&);

inline constexpr FragmentShader lambertShader = [](ShaderContext& context, const FrameUniforms& frame,
                                                   const DrawUniforms& draw) {
    const auto& material = *draw.material;
    Color color = (draw.hasTexcoords && material.texture) ? material.texture->sample(context.uv) : material.baseColor;

    const auto& lighting = *frame.lighting;
    const Color factor =
        lighting.ambientColor + lighting.light.color * std::max(0.f, dot(context.normal, lighting.light.direction));
    color.r *= factor.r;
    color.g *= factor.g;
    color.b *= factor.b;
    context.color = color;
};

inline constexpr FragmentShader unlitShader = [](ShaderContext& context, const FrameUniforms& /*frame*/,
                                                 const DrawUniforms& draw) {
    const auto& material = *draw.material;
    context.color = (draw.hasTexcoords && material.texture) ? material.texture->sample(context.uv) : material.baseColor;
};

inline constexpr FragmentShader normalsShader = [](ShaderContext& context, const FrameUniforms& /*frame*/,
                                                   const DrawUniforms& /*draw*/) {
    context.color = Color((context.normal.x + 1.f) / 2.f, (context.normal.y + 1.f) / 2.f,
                          (context.normal.z + 1.f) / 2.f, 1.f);
};

inline constexpr FragmentShader texcoordsShader = [](ShaderContext& context, const FrameUniforms& /*frame*/,
                                                     const DrawUniforms& draw) {
    context.color = draw.hasTexcoords ? Color(context.uv.x, context.uv.y, 0.0f, 1.0f) : Color(0, 0, 0, 1);
};

/* Screen stages
 *
 * Выполняются после геометрии для каждого пикселя буфера. Стадии получают
 * текущий цвет и могут его заменить. Скайбокс и визуализация глубины —
 * такие же стадии, поэтому отдельных типов шейдеров для них не нужно.
 */

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

inline constexpr ScreenShader normalizeDepthShader = [](const ScreenShaderInput& input,
                                                        const FrameUniforms& /*frame*/) -> Color {
    if (input.minDepth < input.maxDepth && std::isfinite(input.depth)) {
        const float gray = (input.depth - input.minDepth) / (input.maxDepth - input.minDepth);
        return Color(gray, gray, gray, 1.0f);
    }
    return Color(0, 0, 0, 1);
};

/* Pipelines */

struct Pipeline {
    std::string_view name;
    VertexShader vertexShader = defaultVertexShader;
    std::span<const FragmentShader> fragmentShaders{};
    std::span<const ScreenShader> screenShaders{};
    bool depthOnly = false;
};

inline constexpr FragmentShader lambertStages[] = {lambertShader};
inline constexpr FragmentShader unlitStages[] = {unlitShader};
inline constexpr FragmentShader normalsStages[] = {normalsShader};
inline constexpr FragmentShader texcoordsStages[] = {texcoordsShader};

inline constexpr ScreenShader skyboxStages[] = {skyboxShader};
inline constexpr ScreenShader depthStages[] = {normalizeDepthShader};

inline constexpr Pipeline passes[] = {
    // Чтобы добавить режим, опишите стадии выше и добавьте сюда строку:
    // растеризатор и интерфейс подхватят пайплайн автоматически.
    {.name = "Lambert",
     .vertexShader = defaultVertexShader,
     .fragmentShaders = lambertStages,
     .screenShaders = skyboxStages},
    {.name = "Unlit", .vertexShader = defaultVertexShader, .fragmentShaders = unlitStages},
    {.name = "Depth",
     .vertexShader = defaultVertexShader,
     .screenShaders = depthStages,
     .depthOnly = true},
    {.name = "Normals", .vertexShader = defaultVertexShader, .fragmentShaders = normalsStages},
    {.name = "Texcoords", .vertexShader = defaultVertexShader, .fragmentShaders = texcoordsStages},
};

inline constexpr std::size_t passesCount = std::size(passes);

}  // namespace shaders
