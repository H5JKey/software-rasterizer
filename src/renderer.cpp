#include "renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include "frame-buffer.hpp"
#include "math/vectors.hpp"
#include "shaders.hpp"

void Renderer::drawTriangle(FrameBuffer& buffer, const ScreenVertexAttributes& v0, const ScreenVertexAttributes& v1,
                            const ScreenVertexAttributes& v2, float edge012, const Scene::Lighting& lighting,
                            const Material& material, bool hasTexcoords) const {
    const float width = static_cast<float>(buffer.width);
    const float height = static_cast<float>(buffer.height);

    vec2 bboxMin = min(min(v0.position, v1.position), v2.position);
    vec2 bboxMax = max(max(v0.position, v1.position), v2.position);

    bboxMax.x = std::clamp(std::ceil(bboxMax.x), 0.0f, width);
    bboxMax.y = std::clamp(std::ceil(bboxMax.y), 0.0f, height);
    bboxMin.x = std::clamp(std::floor(bboxMin.x), 0.0f, width);
    bboxMin.y = std::clamp(std::floor(bboxMin.y), 0.0f, height);

    const FragmentShaderUniforms uniforms{
        .lighting = lighting,
        .material = material,
    };

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

            if (mode == RenderMode::Depth) {
#pragma omp critical
                if (depth < buffer.depth[idx]) {
                    buffer.depth[idx] = depth;
                }
                continue;
            }
            if (depth >= buffer.depth[idx]) continue;

            float q0 = u * v0.invW;
            float q1 = v * v1.invW;
            float q2 = w * v2.invW;
            float s = 1.0f / (q0 + q1 + q2);
            float p0 = q0 * s;
            float p1 = q1 * s;
            float p2 = q2 * s;

            vec3 normal = normalize(v0.normal * p0 + v1.normal * p1 + v2.normal * p2);
            vec2 uv;
            if (hasTexcoords) uv = v0.texcoord * p0 + v1.texcoord * p1 + v2.texcoord * p2;
            Color pixelColor =
                fragmentShader(
                    FragmentShaderInput{.normal = normal, .uv = uv, .depth = depth, .hasTexcoords = hasTexcoords},
                    uniforms)
                    .color;
#pragma omp critical
            {
                if (depth < buffer.depth[idx]) {
                    buffer.pixels[idx] = pixelColor;

                    buffer.depth[idx] = depth;
                }
            }
        }
    }
}

void Renderer::drawObject(FrameBuffer& buffer, const Object& object, const mat4& VP, const Scene::Lighting& lighting,
                          float zNear) {
    const Mesh& mesh = object.mesh;
    const bool hasTexcoords = mesh.texcoords.size() == mesh.vertices.size();

    clipCoords.resize(mesh.vertices.size());
    transformedNormals.resize(mesh.vertices.size());

    for (std::size_t instance_id = 0; instance_id < object.instancesCount(); ++instance_id) {
        const VertexShaderUniforms uniforms{
            .VP = VP, .M = object.getModel(), .inverseTransposeM = object.getInverseTransposeModel()};

#pragma omp parallel for
        for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
            const auto output = vertexShader(
                VertexShaderInput{.position = mesh.vertices[i], .normal = mesh.normals[i], .instance_id = instance_id},
                uniforms);
            clipCoords[i] = output.position;
            transformedNormals[i] = output.normal;
        }
#pragma omp parallel for
        for (std::size_t tri_idx = 0; tri_idx < mesh.indices.size() / 3; ++tri_idx) {
            const std::size_t i0 = static_cast<std::size_t>(mesh.indices[3 * tri_idx]);
            const std::size_t i1 = static_cast<std::size_t>(mesh.indices[3 * tri_idx + 1]);
            const std::size_t i2 = static_cast<std::size_t>(mesh.indices[3 * tri_idx + 2]);

            if (clipCoords[i0].w < zNear || clipCoords[i1].w < zNear || clipCoords[i2].w < zNear) continue;

            ScreenVertexAttributes attributes[3];
            const std::size_t vertexIndices[] = {i0, i1, i2};
            for (int i = 0; i < 3; ++i) {
                const std::size_t vertexIndex = vertexIndices[i];
                const auto& vertex = clipCoords[vertexIndex];
                const float invW = 1.0f / vertex.w;

                attributes[i].position.x = (vertex.x * invW + 1.0f) * 0.5f * static_cast<float>(buffer.width);
                attributes[i].position.y = (vertex.y * invW + 1.0f) * 0.5f * static_cast<float>(buffer.height);
                attributes[i].depth = vertex.z * invW;
                attributes[i].invW = invW;
                attributes[i].normal = transformedNormals[vertexIndex];
                if (hasTexcoords) attributes[i].texcoord = mesh.texcoords[vertexIndex];
            }

            const float area = edgeFunction(attributes[0].position, attributes[1].position, attributes[2].position);
            if (area < 1e-6f) continue;  // Degenerate triangle or back face.

            drawTriangle(buffer, attributes[0], attributes[1], attributes[2], area, lighting, object.material,
                         hasTexcoords);
        }
    }
}

void Renderer::drawScene(FrameBuffer& buffer, const Scene& scene) {
    const mat4 VP = scene.camera.getPerspectiveMatrix() * scene.camera.getLookAtMatrix();

    for (const auto& object : scene.objects) {
        drawObject(buffer, object, VP, scene.lighting, scene.camera.zNear);
    }

    if (mode == RenderMode::Depth) {
        normalizeDepth(buffer);
    }
}

void Renderer::setRenderMode(RenderMode mode) {
    this->mode = mode;
    switch (mode) {
        case RenderMode::Unlit:
            fragmentShader = defaultShaders::unlitShader;
            break;
        case RenderMode::Shaded:
            fragmentShader = defaultShaders::lambertShader;
            break;
        case RenderMode::Normals:
            fragmentShader = defaultShaders::normalsShader;
            break;
        case RenderMode::Depth:
            fragmentShader = defaultShaders::depthShader;
            break;
        case RenderMode::Texcoords:
            fragmentShader = defaultShaders::texcoordsShader;
            break;
    }
}

void Renderer::normalizeDepth(FrameBuffer& buffer) {
    float minDepth = std::numeric_limits<float>::infinity();
    float maxDepth = -std::numeric_limits<float>::infinity();
    for (float depth : buffer.depth) {
        if (std::isfinite(depth)) {
            minDepth = std::min(minDepth, depth);
            maxDepth = std::max(maxDepth, depth);
        }
    }
    PostProcessingShaderUniforms uniforms{.image = buffer, .minDepth = minDepth, .maxDepth = maxDepth};
#pragma omp parallel for
    for (std::size_t idx = 0; idx < buffer.depth.size(); idx++) {
        buffer.pixels[idx] =
            defaultShaders::normalizeDepthColors(PostProcessingShaderInput{.pixel_idx = idx}, uniforms).color;
    }
}