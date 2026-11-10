module;
#include <algorithm>
#include <cmath>

export module model.material.shaders;
import math;
import model.texture;
export import model.material;

export namespace shaders {

inline constexpr MaterialShader defaultMaterialShader = [](MaterialInput& input, const Material& material) {
    const TextureWithMipLevels* texture = material.textures.empty() ? nullptr : material.textures.front();
    input.color = (input.hasTexcoords && texture) ? texture->sample(input.uv, input.lod) : material.baseColor;
};

inline constexpr MaterialShader rainbowMaterialShader = [](MaterialInput& input, const Material&) {
    const float phase = input.time * 2.0f;
    input.color = Color(0.5f + 0.5f * std::cos(phase), 0.5f + 0.5f * std::cos(phase + 2.09439510f),
                        0.5f + 0.5f * std::cos(phase + 4.18879020f), 1.0f);
};

inline constexpr MaterialShader TerrainTexturesMaterialShader = [](MaterialInput& input, const Material& material) {
    if (!input.hasTexcoords || material.textures.empty()) {
        input.color = material.baseColor;
        return;
    }

    const TextureWithMipLevels* grass = material.textures[0];
    const TextureWithMipLevels* stone = material.textures.size() > 1 ? material.textures[1] : grass;

    const float slope = 1.0f - std::clamp(input.normal.y, 0.0f, 1.0f);

    const float blend = std::clamp((slope - 0.15f) / 0.30f, 0.0f, 1.0f);

    const Color grassColor = grass ? grass->sample(input.uv, input.lod) : material.baseColor;
    const Color stoneColor = stone ? stone->sample(input.uv, input.lod) : material.baseColor;

    input.color = grassColor * (1.0f - blend) + stoneColor * blend;
};
}  // namespace shaders
