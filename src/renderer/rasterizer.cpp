module;
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

module renderer.rasterizer;

import renderer.framebuffer;
import model.material;
import model.object;
import math;
import scene.scene;
import renderer.shaders;

void Rasterizer::drawTriangle(Framebuffer& buffer, const ScreenVertexAttributes& v0, const ScreenVertexAttributes& v1,
                              const ScreenVertexAttributes& v2, float edge012,
                              const shaders::FrameUniforms& frameUniforms,
                              const shaders::DrawUniforms& drawUniforms) const {
    const float width = static_cast<float>(buffer.width);
    const float height = static_cast<float>(buffer.height);

    vec2 bboxMin = min(min(v0.position, v1.position), v2.position);
    vec2 bboxMax = max(max(v0.position, v1.position), v2.position);

    bboxMax.x = std::clamp(std::ceil(bboxMax.x), 0.0f, width);
    bboxMax.y = std::clamp(std::ceil(bboxMax.y), 0.0f, height);
    bboxMin.x = std::clamp(std::floor(bboxMin.x), 0.0f, width);
    bboxMin.y = std::clamp(std::floor(bboxMin.y), 0.0f, height);

    const auto& pipeline = currentPipeline();
    const bool hasFragmentShaders = !pipeline.fragmentShaders.empty();

    for (int py = static_cast<int>(bboxMin.y); py < static_cast<int>(bboxMax.y); ++py) {
        for (int px = static_cast<int>(bboxMin.x); px < static_cast<int>(bboxMax.x); ++px) {
            vec2 center(px + 0.5f, py + 0.5f);

            float e12p = edgeFunction(v1.position, v2.position, center);
            float e20p = edgeFunction(v2.position, v0.position, center);
            float e01p = edgeFunction(v0.position, v1.position, center);

            if (e12p * edge012 < 0.0f || e20p * edge012 < 0.0f || e01p * edge012 < 0.0f) continue;

            float u = e12p / edge012;
            float v = e20p / edge012;
            float w = e01p / edge012;

            float depth = v0.depth * u + v1.depth * v + v2.depth * w;

            std::size_t idx = static_cast<std::size_t>(py) * buffer.width + static_cast<std::size_t>(px);

            if (depth < buffer.depth[idx])
                buffer.depth[idx] = depth;
            else
                continue;

            if (!hasFragmentShaders) continue;

            float q0 = u * v0.invW;
            float q1 = v * v1.invW;
            float q2 = w * v2.invW;
            float s = 1.0f / (q0 + q1 + q2);
            float p0 = q0 * s;
            float p1 = q1 * s;
            float p2 = q2 * s;

            shaders::ShaderContext context{
                .worldPosition = v0.worldPosition * p0 + v1.worldPosition * p1 + v2.worldPosition * p2,
                .normal = normalize(v0.normal * p0 + v1.normal * p1 + v2.normal * p2),
                .uv = v0.texcoord * p0 + v1.texcoord * p1 + v2.texcoord * p2,
                .depth = depth,
            };

            for (const shaders::FragmentShader stage : pipeline.fragmentShaders) {
                stage(context, frameUniforms, drawUniforms);
            }

            buffer.pixels[idx] = context.color;
        }
    }
}

void Rasterizer::drawObject(Framebuffer& buffer, const Object& object, const shaders::FrameUniforms& frameUniforms,
                            float zNear) {
    const Mesh& mesh = object.mesh;
    const bool hasTexcoords = mesh.texcoords.size() == mesh.vertices.size();

    clipCoords.resize(mesh.vertices.size());
    transformedNormals.resize(mesh.vertices.size());
    transformedWorldPositions.resize(mesh.vertices.size());

    const auto& modelMatrices = object.getModel();
    const auto& inverseTransposeModelMatrices = object.getInverseTransposeModel();

    struct ClipVertex {
        vec4 position;
        vec3 worldPosition;
        vec3 normal;
        vec2 texcoord;
    };

    auto texcoordAt = [&](std::size_t index) { return hasTexcoords ? mesh.texcoords[index] : vec2(); };

    auto toScreen = [&](const ClipVertex& clip) -> ScreenVertexAttributes {
        ScreenVertexAttributes output{};
        float invW = 1.f / clip.position.w;
        output.position.x = (clip.position.x * invW + 1.0f) * 0.5f * static_cast<float>(buffer.width);
        output.position.y = (clip.position.y * invW + 1.0f) * 0.5f * static_cast<float>(buffer.height);
        output.depth = clip.position.z * invW;
        output.invW = invW;
        output.normal = clip.normal;
        output.worldPosition = clip.worldPosition;
        output.texcoord = clip.texcoord;
        return output;
    };

    for (std::size_t instance_id = 0; instance_id < object.instancesCount(); ++instance_id) {
        const shaders::DrawUniforms drawUniforms{.M = &modelMatrices[instance_id],
                                                 .inverseTransposeM = &inverseTransposeModelMatrices[instance_id],
                                                 .material = &object.material,
                                                 .instanceId = instance_id,
                                                 .hasTexcoords = hasTexcoords};

#pragma omp parallel for
        for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
            const auto output = currentPipeline().vertexShader(
                shaders::VertexShaderInput{
                    .position = mesh.vertices[i],
                    .normal = mesh.normals[i],
                    .uv = hasTexcoords ? mesh.texcoords[i] : vec2(),
                },
                frameUniforms, drawUniforms);
            clipCoords[i] = output.clipPosition;
            transformedNormals[i] = output.normal;
            transformedWorldPositions[i] = output.worldPosition;
        }

        for (std::size_t tri_idx = 0; tri_idx < mesh.indices.size() / 3; ++tri_idx) {
            const std::size_t i0 = static_cast<std::size_t>(mesh.indices[3 * tri_idx]);
            const std::size_t i1 = static_cast<std::size_t>(mesh.indices[3 * tri_idx + 1]);
            const std::size_t i2 = static_cast<std::size_t>(mesh.indices[3 * tri_idx + 2]);
            const std::size_t vertexIndices[] = {i0, i1, i2};
            if (clipCoords[i0].w < zNear || clipCoords[i1].w < zNear || clipCoords[i2].w < zNear) {
                ClipVertex polygon[4];
                std::size_t polygonSize = 0;
                for (int i = 0; i < 3; i++) {
                    const std::size_t a = vertexIndices[i];
                    const std::size_t b = vertexIndices[(i + 1) % 3];
                    const float da = clipCoords[a].w + clipCoords[a].z;
                    const float db = clipCoords[b].w + +clipCoords[b].z;

                    if (da >= 0) {
                        polygon[polygonSize++] = ClipVertex{.position = clipCoords[a],
                                                            .worldPosition = transformedWorldPositions[a],
                                                            .normal = transformedNormals[a],
                                                            .texcoord = texcoordAt(a)};
                    }
                    if ((da >= 0) != (db >= 0)) {
                        const float t = da / (da - db);
                        polygon[polygonSize++] = ClipVertex{
                            .position = clipCoords[a] + (clipCoords[b] - clipCoords[a]) * t,
                            .worldPosition = transformedWorldPositions[a] +
                                             (transformedWorldPositions[b] - transformedWorldPositions[a]) * t,
                            .normal = transformedNormals[a] + (transformedNormals[b] - transformedNormals[a]) * t,
                            .texcoord = texcoordAt(a) + (texcoordAt(b) - texcoordAt(a)) * t,
                        };
                    }
                }

                if (polygonSize < 3) continue;

                for (std::size_t i = 1; i + 1 < polygonSize; ++i) {
                    const ScreenVertexAttributes attributes[3] = {
                        toScreen(polygon[0]),
                        toScreen(polygon[i]),
                        toScreen(polygon[i + 1]),
                    };

                    const float area =
                        edgeFunction(attributes[0].position, attributes[1].position, attributes[2].position);
                    if (area < 1e-6f) continue;

                    drawTriangle(buffer, attributes[0], attributes[1], attributes[2], area, frameUniforms,
                                 drawUniforms);
                }
                continue;
            } else {
                const ScreenVertexAttributes attributes[3] = {
                    toScreen(ClipVertex{.position = clipCoords[i0],
                                        .worldPosition = transformedWorldPositions[i0],
                                        .normal = transformedNormals[i0],
                                        .texcoord = texcoordAt(i0)}),
                    toScreen(ClipVertex{.position = clipCoords[i1],
                                        .worldPosition = transformedWorldPositions[i1],
                                        .normal = transformedNormals[i1],
                                        .texcoord = texcoordAt(i1)}),
                    toScreen(ClipVertex{.position = clipCoords[i2],
                                        .worldPosition = transformedWorldPositions[i2],
                                        .normal = transformedNormals[i2],
                                        .texcoord = texcoordAt(i2)}),
                };

                const float area = edgeFunction(attributes[0].position, attributes[1].position, attributes[2].position);
                if (area < 1e-6f) continue;  // Degenerate triangle or back face.

                drawTriangle(buffer, attributes[0], attributes[1], attributes[2], area, frameUniforms, drawUniforms);
            }
        }
    }
}

void Rasterizer::runScreenStages(Framebuffer& buffer, const shaders::FrameUniforms& frameUniforms) {
    const auto& pipeline = currentPipeline();
    if (pipeline.screenShaders.empty()) return;

    float minDepth = std::numeric_limits<float>::infinity();
    float maxDepth = -std::numeric_limits<float>::infinity();
    for (float depth : buffer.depth) {
        if (std::isfinite(depth)) {
            minDepth = std::min(minDepth, depth);
            maxDepth = std::max(maxDepth, depth);
        }
    }

#pragma omp parallel for
    for (std::size_t idx = 0; idx < buffer.depth.size(); idx++) {
        const int x = static_cast<int>(idx % static_cast<std::size_t>(buffer.width));
        const int y = static_cast<int>(idx / static_cast<std::size_t>(buffer.width));

        shaders::ScreenShaderInput input{
            .x = x,
            .y = y,
            .ndcX = 2.0f * (static_cast<float>(x) + 0.5f) / static_cast<float>(buffer.width) - 1.0f,
            .ndcY = 2.0f * (static_cast<float>(y) + 0.5f) / static_cast<float>(buffer.height) - 1.0f,
            .depth = buffer.depth[idx],
            .color = buffer.pixels[idx],
            .minDepth = minDepth,
            .maxDepth = maxDepth,
        };

        for (const shaders::ScreenShader stage : pipeline.screenShaders) {
            input.color = stage(input, frameUniforms);
        }

        buffer.pixels[idx] = input.color;
    }
}

void Rasterizer::drawScene(Framebuffer& buffer, const Scene& scene) {
    const mat4 VP = scene.camera.getPerspectiveMatrix() * scene.camera.getLookAtMatrix();
    const mat4 inverseVP = VP.inverse();
    const vec3 cameraPosition = scene.camera.getEye();

    const shaders::FrameUniforms frameUniforms{.VP = &VP,
                                               .inverseVP = &inverseVP,
                                               .cameraPosition = &cameraPosition,
                                               .lighting = &scene.lighting,
                                               .skybox = &scene.skybox,
                                               .zNear = scene.camera.zNear,
                                               .zFar = scene.camera.zFar};

    for (const auto& object : scene.getAllObjects()) {
        drawObject(buffer, object, frameUniforms, scene.camera.zNear);
    }

    runScreenStages(buffer, frameUniforms);
}
