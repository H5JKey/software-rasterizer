#include "renderer.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "frame-buffer.hpp"
#include "math/vectors.hpp"

void Renderer::drawTriangle(FrameBuffer& buffer, const VertexAttributes& v0, const VertexAttributes& v1,
                            const VertexAttributes& v2, float edge012, const Scene::Lighting& lighting,
                            const Material& material) const {
    float width = static_cast<float>(buffer.width);
    float height = static_cast<float>(buffer.height);

    vec2 bboxMin = min(min(v0.position, v1.position), v2.position);
    vec2 bboxMax = max(max(v0.position, v1.position), v2.position);

    bboxMax.x = std::min(std::max(std::ceil(bboxMax.x), 0.f), width);
    bboxMax.y = std::min(std::max(std::ceil(bboxMax.y), 0.f), height);
    bboxMin.x = std::min(std::max(std::floor(bboxMin.x), 0.f), width);
    bboxMin.y = std::min(std::max(std::floor(bboxMin.y), 0.f), height);

    for (int py = static_cast<int>(bboxMin.y); py < static_cast<int>(bboxMax.y); py++) {
        for (int px = static_cast<int>(bboxMin.x); px < static_cast<int>(bboxMax.x); px++) {
            vec2 center(px + 0.5f, py + 0.5f);

            float e12p = edgeFunction(v1.position, v2.position, center);
            float e20p = edgeFunction(v2.position, v0.position, center);
            float e01p = edgeFunction(v0.position, v1.position, center);

            if (e12p * edge012 < 0 || e20p * edge012 < 0 || e01p * edge012 < 0) continue;

            float u = e12p / edge012;
            float v = e20p / edge012;
            float w = e01p / edge012;

            float depth = v0.depth * u + v1.depth * v + v2.depth * w;
            std::size_t idx = py * buffer.width + px;
            if (depth < buffer.depth[idx]) {
                float q0 = u * v0.depth * v0.invW;
                float q1 = u * v1.depth * v1.invW;
                float q2 = u * v1.depth * v1.invW;
                float s = 1.0f / (q0 + q1 + q2);
                q0 *= s;
                q1 *= s;
                q2 *= s;
                Color color;
                vec3 normal = normalize(v0.normal * q0 + v1.normal * q1 + v2.normal * q2);
                vec2 uv = v0.texcoord * q0 + v1.texcoord * q1 + v2.texcoord * q2;
                if (material.texture) {
                    color = material.texture->sample(uv);
                } else {
                    color = material.baseColor;
                }
                color = lighting.ambientColor + color * std::max(0.f, dot(normal, lighting.light.direction));

                buffer.pixels[idx] = color;
                buffer.depth[idx] = depth;
                buffer.normals[idx] =
                    Color(normal.x * 0.5f + 0.5f, normal.y * 0.5f + 0.5f, normal.z * 0.5f + 0.5f, 1.0f);
            }
        }
    }
}

void Renderer::drawMesh(FrameBuffer& buffer, const Mesh& mesh, const mat4& M, const mat4& VP, float zNear,
                        const Scene::Lighting& lighting, const Material& material) const {
    std::vector<vec4> clipCoords;
    clipCoords.reserve(mesh.vertices.size());

    mat4 MVP = VP * M;
    mat3 normalMat = M.upperLeft<3>().inverse().transpose();

    for (const auto& v : mesh.vertices) clipCoords.emplace_back(MVP * vec4(v.x, v.y, v.z, 1.0f));

    for (size_t tri_idx = 0; tri_idx < mesh.indices.size() / 3; tri_idx++) {
        if (clipCoords[mesh.indices[3 * tri_idx]].w < zNear || clipCoords[mesh.indices[3 * tri_idx + 1]].w < zNear ||
            clipCoords[mesh.indices[3 * tri_idx + 2]].w < zNear)
            continue;
        VertexAttributes attributes[3];
        for (int i = 0; i < 3; i++) {
            const auto& v = clipCoords[mesh.indices[3 * tri_idx + i]];
            float invW = 1.0f / v.w;
            attributes[i].position.x = (v.x * invW + 1) * 0.5f * buffer.width;
            attributes[i].position.y = (v.y * invW + 1) * 0.5f * buffer.height;
            attributes[i].depth = v.z * invW;
            attributes[i].invW = invW;
            attributes[i].texcoord = mesh.texcoords[mesh.indices[3 * tri_idx + i]];
            vec3 normal = mesh.normals[mesh.indices[3 * tri_idx + i]];
            attributes[i].normal = normalMat * normal;
        }
        float area = edgeFunction(attributes[0].position, attributes[1].position, attributes[2].position);
        if (area < 1e-6) continue; /* degenerate triangle or not visible */
        drawTriangle(buffer, attributes[0], attributes[1], attributes[2], area, lighting, material);
    }
}

void Renderer::drawScene(FrameBuffer& buffer, const Scene& scene) const {
    mat4 VP = scene.camera.getPerspectiveMatrix() * scene.camera.getLookAtMatrix();
    for (const auto& object : scene.objects) {
        drawMesh(buffer, object.mesh, object.getModelMatrix(), VP, scene.camera.zNear, scene.lighting, object.material);
    }
}