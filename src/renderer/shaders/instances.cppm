module;
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

export module renderer.shaders:instances;

import math;
import renderer.image;
import model.material;
import model.material.shaders;
import model.texture;
import scene.skybox;
import scene.terrain;
import :contract;

export namespace shaders {

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

inline constexpr FragmentShader shadowMapShader = [](ShaderContext& context, const FrameUniforms& frame,
                                                     const DrawUniforms& draw) {
    float shadow = 1.0f;

    const auto& lighting = *frame.lighting;

    const float ndotl = std::max(0.f, dot(context.normal, lighting.light.direction));

    if (frame.shadowMap) {
        const ShadowMap& sm = *frame.shadowMap;

        const vec4 lightClip =
            sm.lightVP * vec4(context.worldPosition.x, context.worldPosition.y, context.worldPosition.z, 1.0f);
        const float invW = 1.0f / lightClip.w;
        const vec3 ndc = vec3(lightClip.x * invW, lightClip.y * invW, lightClip.z * invW);

        if (ndc.x >= -1.0f && ndc.x <= 1.0f && ndc.y >= -1.0f && ndc.y <= 1.0f) {
            vec2 texel = vec2(1.0f / static_cast<float>(sm.buffer.width), 1.0f / static_cast<float>(sm.buffer.height));
            float lit = 0.0f;
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    float sampleNdcX = ndc.x + static_cast<float>(dx) * texel.x;
                    float sampleNdcY = ndc.y + static_cast<float>(dy) * texel.y;

                    int x =
                        std::clamp(static_cast<int>((sampleNdcX + 1.0f) * 0.5f * static_cast<float>(sm.buffer.width)),
                                   0, sm.buffer.width - 1);
                    int y =
                        std::clamp(static_cast<int>((sampleNdcY + 1.0f) * 0.5f * static_cast<float>(sm.buffer.height)),
                                   0, sm.buffer.height - 1);

                    const float closest = sm.buffer.depth()[static_cast<std::size_t>(y) * sm.buffer.width + x];

                    lit += (ndc.z - sm.bias * (1.0 - ndotl) * 5 > closest) ? 0.0f : 1.0f;
                }
            }
            shadow = lit / 25.0f;
        }
    }

    const Color factor = lighting.ambientColor + lighting.light.color * ndotl * shadow;

    Color color = context.color;
    color.r *= factor.r;
    color.g *= factor.g;
    color.b *= factor.b;
    context.color = color;
};

inline constexpr FragmentShader applyMaterialShader = [](ShaderContext& context, const FrameUniforms& frame,
                                                         const DrawUniforms& draw) {
    const Material& material = *draw.material;

    const float distance =
        2.0f * frame.zNear * frame.zFar / (frame.zFar + frame.zNear - context.depth * (frame.zFar - frame.zNear));
    const float lod = std::log2(std::max(distance, 1e-4f));

    const auto& lighting = *frame.lighting;
    MaterialInput input{
        .worldPosition = context.worldPosition,
        .normal = context.normal,
        .uv = context.uv,
        .depth = context.depth,
        .lod = lod,
        .time = frame.time,
        .hasTexcoords = draw.hasTexcoords,
        .instanceId = draw.instanceId,
        .color = context.color,
        .ambientColor = lighting.ambientColor,
        .lightColor = lighting.light.color,
        .lightDirection = lighting.light.direction,
    };

    if (material.shader) {
        material.shader(input, material);
    } else {
        defaultMaterialShader(input, material);
    }

    context.color = input.color;
};

inline constexpr FragmentShader mipLevelShader = [](ShaderContext& context, const FrameUniforms& frame,
                                                    const DrawUniforms& draw) {
    const auto& material = *draw.material;
    const TextureWithMipLevels* texture = material.textures.empty() ? nullptr : material.textures.front();
    if (!draw.hasTexcoords || texture == nullptr) {
        context.color = Color(0, 0, 0, 1);
        return;
    }

    float distance =
        2.0f * frame.zNear * frame.zFar / (frame.zFar + frame.zNear - context.depth * (frame.zFar - frame.zNear));
    float lod = std::log2(std::max(distance, 1e-4f));

    std::size_t levels = texture->mipLevels.size();
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

inline constexpr ScreenShader fxaaShader = [](const ScreenShaderInput& input, const FrameUniforms& frame) -> Color {
    if (!frame.fxaaEnabled || input.source == nullptr || input.luma == nullptr) return input.color;

    const Image& source = *input.source;
    const std::vector<float>& luma = *input.luma;
    const int width = source.width;
    const int height = source.height;
    const int x = input.x;
    const int y = input.y;

    if (x <= 0 || y <= 0 || x >= width - 1 || y >= height - 1) return input.color;

    auto lumaAt = [&](int sx, int sy) {
        sx = std::clamp(sx, 0, width - 1);
        sy = std::clamp(sy, 0, height - 1);
        return luma[static_cast<std::size_t>(sy) * static_cast<std::size_t>(width) + static_cast<std::size_t>(sx)];
    };

    auto lumaBilinear = [&](float sx, float sy) {
        const float fx = sx - 0.5f;
        const float fy = sy - 0.5f;
        const int ix = static_cast<int>(std::floor(fx));
        const int iy = static_cast<int>(std::floor(fy));
        const float tx = fx - static_cast<float>(ix);
        const float ty = fy - static_cast<float>(iy);
        if (tx == 0.0f && ty == 0.0f) return lumaAt(ix, iy);
        const float c00 = lumaAt(ix, iy);
        const float c10 = lumaAt(ix + 1, iy);
        const float c01 = lumaAt(ix, iy + 1);
        const float c11 = lumaAt(ix + 1, iy + 1);
        return c00 * (1.0f - tx) * (1.0f - ty) + c10 * tx * (1.0f - ty) + c01 * (1.0f - tx) * ty + c11 * tx * ty;
    };

    auto sampleColor = [&](float sx, float sy) {
        const float fx = sx - 0.5f;
        const float fy = sy - 0.5f;
        const int ix = static_cast<int>(std::floor(fx));
        const int iy = static_cast<int>(std::floor(fy));
        const float tx = fx - static_cast<float>(ix);
        const float ty = fy - static_cast<float>(iy);
        if (tx == 0.0f && ty == 0.0f) {
            const int cx = std::clamp(ix, 0, width - 1);
            const int cy = std::clamp(iy, 0, height - 1);
            return source
                .pixels[static_cast<std::size_t>(cy) * static_cast<std::size_t>(width) + static_cast<std::size_t>(cx)];
        }
        const int left = std::clamp(ix, 0, width - 1);
        const int right = std::clamp(ix + 1, 0, width - 1);
        const int up = std::clamp(iy, 0, height - 1);
        const int down = std::clamp(iy + 1, 0, height - 1);
        const std::size_t row0 = static_cast<std::size_t>(up) * static_cast<std::size_t>(width);
        const std::size_t row1 = static_cast<std::size_t>(down) * static_cast<std::size_t>(width);
        const Color c00 = source.pixels[row0 + static_cast<std::size_t>(left)];
        const Color c10 = source.pixels[row0 + static_cast<std::size_t>(right)];
        const Color c01 = source.pixels[row1 + static_cast<std::size_t>(left)];
        const Color c11 = source.pixels[row1 + static_cast<std::size_t>(right)];
        return c00 * (1.0f - tx) * (1.0f - ty) + c10 * tx * (1.0f - ty) + c01 * (1.0f - tx) * ty + c11 * tx * ty;
    };

    constexpr int FXAA_QUALITY_PS = 5;
    constexpr float FXAA_QUALITY_P[FXAA_QUALITY_PS] = {1.0f, 1.5f, 2.0f, 4.0f, 12.0f};
    constexpr float FXAA_SUBPIX = 0.75f;
    constexpr float FXAA_EDGE_THRESHOLD = 0.166f;
    constexpr float FXAA_EDGE_THRESHOLD_MIN = 0.0833f;

    const float lumaM = lumaAt(x, y);
    const float lumaS = lumaAt(x, y + 1);
    const float lumaE = lumaAt(x + 1, y);
    const float lumaN = lumaAt(x, y - 1);
    const float lumaW = lumaAt(x - 1, y);

    const float rangeMax = std::max(std::max(lumaS, lumaM), std::max(std::max(lumaE, lumaN), lumaW));
    const float rangeMin = std::min(std::min(lumaS, lumaM), std::min(std::min(lumaE, lumaN), lumaW));
    const float range = rangeMax - rangeMin;
    if (range < std::max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD)) return input.color;

    const float lumaNW = lumaAt(x - 1, y - 1);
    const float lumaSE = lumaAt(x + 1, y + 1);
    const float lumaNE = lumaAt(x + 1, y - 1);
    const float lumaSW = lumaAt(x - 1, y + 1);

    const float lumaNS = lumaN + lumaS;
    const float lumaWE = lumaW + lumaE;
    const float subpixRcpRange = 1.0f / range;
    const float subpixNSWE = lumaNS + lumaWE;
    const float edgeHorz1 = (-2.0f * lumaM) + lumaNS;
    const float edgeVert1 = (-2.0f * lumaM) + lumaWE;

    const float lumaNESE = lumaNE + lumaSE;
    const float lumaNWNE = lumaNW + lumaNE;
    const float edgeHorz2 = (-2.0f * lumaE) + lumaNESE;
    const float edgeVert2 = (-2.0f * lumaN) + lumaNWNE;

    const float lumaNWSW = lumaNW + lumaSW;
    const float lumaSWSE = lumaSW + lumaSE;
    const float edgeHorz4 = (std::abs(edgeHorz1) * 2.0f) + std::abs(edgeHorz2);
    const float edgeVert4 = (std::abs(edgeVert1) * 2.0f) + std::abs(edgeVert2);
    const float edgeHorz3 = (-2.0f * lumaW) + lumaNWSW;
    const float edgeVert3 = (-2.0f * lumaS) + lumaSWSE;
    const float edgeHorz = std::abs(edgeHorz3) + edgeHorz4;
    const float edgeVert = std::abs(edgeVert3) + edgeVert4;

    const float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float lengthSign = 1.0f;
    const bool horzSpan = edgeHorz >= edgeVert;
    const float subpixA = subpixNSWE * 2.0f + subpixNWSWNESE;

    const float lumaNAdj = horzSpan ? lumaN : lumaW;
    const float lumaSAdj = horzSpan ? lumaS : lumaE;
    const float subpixB = (subpixA * (1.0f / 12.0f)) - lumaM;

    const float gradientN = lumaNAdj - lumaM;
    const float gradientS = lumaSAdj - lumaM;
    float lumaNN = lumaNAdj + lumaM;
    const float lumaSS = lumaSAdj + lumaM;
    const bool pairN = std::abs(gradientN) >= std::abs(gradientS);
    const float gradient = std::max(std::abs(gradientN), std::abs(gradientS));
    if (pairN) lengthSign = -lengthSign;
    const float subpixC = std::clamp(std::abs(subpixB) * subpixRcpRange, 0.0f, 1.0f);

    const float offNX = horzSpan ? 1.0f : 0.0f;
    const float offNY = horzSpan ? 0.0f : 1.0f;

    const float posMx = static_cast<float>(x) + 0.5f;
    const float posMy = static_cast<float>(y) + 0.5f;
    float posBx = posMx;
    float posBy = posMy;
    if (!horzSpan) posBx += lengthSign * 0.5f;
    if (horzSpan) posBy += lengthSign * 0.5f;

    float posNx = posBx - offNX * FXAA_QUALITY_P[0];
    float posNy = posBy - offNY * FXAA_QUALITY_P[0];
    float posPx = posBx + offNX * FXAA_QUALITY_P[0];
    float posPy = posBy + offNY * FXAA_QUALITY_P[0];

    const float subpixD = (-2.0f * subpixC) + 3.0f;
    float lumaEndN = lumaBilinear(posNx, posNy);
    const float subpixE = subpixC * subpixC;
    float lumaEndP = lumaBilinear(posPx, posPy);

    if (!pairN) lumaNN = lumaSS;
    const float gradientScaled = gradient * 0.25f;
    const float lumaMM = lumaM - lumaNN * 0.5f;
    const float subpixF = subpixD * subpixE;
    const bool lumaMLTZero = lumaMM < 0.0f;

    lumaEndN -= lumaNN * 0.5f;
    lumaEndP -= lumaNN * 0.5f;
    bool doneN = std::abs(lumaEndN) >= gradientScaled;
    bool doneP = std::abs(lumaEndP) >= gradientScaled;

    for (int i = 1; i < FXAA_QUALITY_PS; ++i) {
        if (doneN && doneP) break;
        if (!doneN) {
            posNx -= offNX * FXAA_QUALITY_P[i];
            posNy -= offNY * FXAA_QUALITY_P[i];
        }
        if (!doneP) {
            posPx += offNX * FXAA_QUALITY_P[i];
            posPy += offNY * FXAA_QUALITY_P[i];
        }
        if (i == FXAA_QUALITY_PS - 1) break;
        if (!doneN) {
            lumaEndN = lumaBilinear(posNx, posNy) - lumaNN * 0.5f;
            doneN = std::abs(lumaEndN) >= gradientScaled;
        }
        if (!doneP) {
            lumaEndP = lumaBilinear(posPx, posPy) - lumaNN * 0.5f;
            doneP = std::abs(lumaEndP) >= gradientScaled;
        }
    }

    const float dstN = horzSpan ? posMx - posNx : posMy - posNy;
    const float dstP = horzSpan ? posPx - posMx : posPy - posMy;

    const bool goodSpanN = (lumaEndN < 0.0f) != lumaMLTZero;
    const float spanLength = dstP + dstN;
    const bool goodSpanP = (lumaEndP < 0.0f) != lumaMLTZero;
    const float spanLengthRcp = 1.0f / spanLength;

    const bool directionN = dstN < dstP;
    const float dst = std::min(dstN, dstP);
    const bool goodSpan = directionN ? goodSpanN : goodSpanP;
    const float subpixG = subpixF * subpixF;
    const float pixelOffset = (dst * (-spanLengthRcp)) + 0.5f;
    const float subpixH = subpixG * FXAA_SUBPIX;

    const float pixelOffsetGood = goodSpan ? pixelOffset : 0.0f;
    const float pixelOffsetSubpix = std::max(pixelOffsetGood, subpixH);
    const float finalX = posMx + (horzSpan ? 0.0f : pixelOffsetSubpix * lengthSign);
    const float finalY = posMy + (horzSpan ? pixelOffsetSubpix * lengthSign : 0.0f);
    return sampleColor(finalX, finalY);
};

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

}  // namespace shaders
