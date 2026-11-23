module;
#include <cstddef>
#include <iterator>
#include <span>
#include <string_view>

export module renderer.shaders:pipeline;

import :contract;
import :instances;

export namespace shaders {

struct Pipeline {
    std::string_view name;
    std::span<const FragmentShader> fragmentShaders{};
    std::span<const ScreenShader> screenShaders{};
};

inline constexpr FragmentShader lightingStages[] = {applyMaterialShader, shadowMapShader};
inline constexpr FragmentShader unlitStages[] = {applyMaterialShader};
inline constexpr FragmentShader normalsStages[] = {normalsShader};
inline constexpr FragmentShader texcoordsStages[] = {texcoordsShader};
inline constexpr FragmentShader mipLevelStages[] = {mipLevelShader};

inline constexpr ScreenShader screenStage[] = {skyboxShader, fxaaShader};
inline constexpr ScreenShader depthStages[] = {normalizeDepthShader};

inline constexpr Pipeline passes[] = {
    {.name = "Main", .fragmentShaders = lightingStages, .screenShaders = screenStage},
    {.name = "Unlit", .fragmentShaders = unlitStages},
    {.name = "Depth", .screenShaders = depthStages},
    {.name = "Normals", .fragmentShaders = normalsStages},
    {.name = "Texcoords", .fragmentShaders = texcoordsStages},
    {.name = "MipLevels", .fragmentShaders = mipLevelStages},
};

inline constexpr std::size_t passesCount = std::size(passes);

}  // namespace shaders
