#include "renderer.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "frame-buffer.hpp"

void Renderer::drawTriangle(FrameBuffer& buffer, vec2 v0, vec2 v1, vec2 v2, Color v0Color, Color v1Color,
                            Color v2Color) const {
    float edge012 = edgeFunction(v0, v1, v2);
    if (fabs(edge012) < 0.001) return;

    float width = static_cast<float>(buffer.width);
    float height = static_cast<float>(buffer.height);

    vec2 bboxMin = min(min(v0, v1), v2);
    vec2 bboxMax = max(max(v0, v1), v2);

    bboxMax.x = std::min(std::max(std::ceil(bboxMax.x), 0.f), width);
    bboxMax.y = std::min(std::max(std::ceil(bboxMax.y), 0.f), height);
    bboxMin.x = std::min(std::max(std::floor(bboxMin.x), 0.f), width);
    bboxMin.y = std::min(std::max(std::floor(bboxMin.y), 0.f), height);

    for (int py = static_cast<int>(bboxMin.y); py < static_cast<int>(bboxMax.y); py++) {
        for (int px = static_cast<int>(bboxMin.x); px < static_cast<int>(bboxMax.x); px++) {
            vec2 center(px + 0.5f, py + 0.5f);

            float e12p = edgeFunction(v1, v2, center);
            float e20p = edgeFunction(v2, v0, center);
            float e01p = edgeFunction(v0, v1, center);

            if (e12p * edge012 < 0 || e20p * edge012 < 0 || e01p * edge012 < 0) continue;

            float u = e12p / edge012;
            float v = e20p / edge012;
            float w = e01p / edge012;

            buffer.pixels[py * buffer.width + px] = v0Color * u + v1Color * v + v2Color * w;
        }
    }
}

void Renderer::drawMesh(FrameBuffer& buffer, const Mesh& mesh, const mat4& MVP, float zNear) const {
    std::vector<vec4> clipCoords;
    clipCoords.reserve(mesh.vertices.size());

    for (const auto& v : mesh.vertices) clipCoords.emplace_back(MVP * vec4(v.x, v.y, v.z, 1.0f));

    for (size_t tri_idx = 0; tri_idx < mesh.indices.size() / 3; tri_idx++) {
        if (clipCoords[mesh.indices[3 * tri_idx]].w < zNear || clipCoords[mesh.indices[3 * tri_idx + 1]].w < zNear ||
            clipCoords[mesh.indices[3 * tri_idx + 2]].w < zNear)
            continue;

        vec2 verticesFramebufferCoords[3];
        float verticesDepth[3];
        for (int i = 0; i < 3; i++) {
            const auto& v = clipCoords[mesh.indices[3 * tri_idx + i]];
            float iw = 1.0f / v.w;
            verticesFramebufferCoords[i].x = (v.x * iw + 1) * 0.5f * buffer.width;
            verticesFramebufferCoords[i].y = (v.y * iw + 1) * 0.5f * buffer.height;
            verticesDepth[i] = v.z * iw;
        }
        drawTriangle(buffer, verticesFramebufferCoords[0], verticesFramebufferCoords[1], verticesFramebufferCoords[2],
                     {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
    }
}

void Renderer::drawObjects(FrameBuffer& buffer, const std::vector<Object>& objects, const Camera& camera) const {
    mat4 VP = camera.getPerspectiveMatrix() * camera.getLookAtMatrix();
    for (const auto& object : objects) {
        mat4 MVP = VP * object.transform;
        drawMesh(buffer, object.mesh, MVP, camera.zNear);
    }
}