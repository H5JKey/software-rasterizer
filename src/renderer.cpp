#include "renderer.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "frame-buffer.hpp"
#include "math/vectors.hpp"
#include "shaders.hpp"

void Renderer::drawTriangle(FrameBuffer& buffer, const ScreenVertexAttributes& v0, const ScreenVertexAttributes& v1,
                            const ScreenVertexAttributes& v2, float edge012, const Scene::Lighting& lighting,
                            const Material& material, bool hasTexcoords) const {
    float width = static_cast<float>(buffer.width);
    float height = static_cast<float>(buffer.height);

    vec2 bboxMin = min(min(v0.position, v1.position), v2.position);
    vec2 bboxMax = max(max(v0.position, v1.position), v2.position);

    bboxMax.x = std::min(std::max(std::ceil(bboxMax.x), 0.f), width);
    bboxMax.y = std::min(std::max(std::ceil(bboxMax.y), 0.f), height);
    bboxMin.x = std::min(std::max(std::floor(bboxMin.x), 0.f), width);
    bboxMin.y = std::min(std::max(std::floor(bboxMin.y), 0.f), height);

    FragmentShaderUniforms uniforms{.lighting = lighting};
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
                buffer.depth[idx] = depth;
                float q0 = u * v0.invW;
                float q1 = v * v1.invW;
                float q2 = w * v2.invW;
                float s = 1.0f / (q0 + q1 + q2);
                q0 *= s;
                q1 *= s;
                q2 *= s;
                vec3 normal = normalize(v0.normal * q0 + v1.normal * q1 + v2.normal * q2);
                vec2 uv;
                if (hasTexcoords) uv = v0.texcoord * q0 + v1.texcoord * q1 + v2.texcoord * q2;
                buffer.pixels[idx] = fragmentShader(FragmentShaderInput{.normal = normal,
                                                                        .material = material,
                                                                        .uv = uv,
                                                                        .depth = depth,
                                                                        .hasTexcoords = hasTexcoords},
                                                    uniforms)
                                         .color;
            }
        }
    }
}

void Renderer::drawObject(FrameBuffer& buffer, const Object& object, const mat4& VP, const Scene::Lighting& lighting,
                          float zNear) const {
    const Mesh& mesh = object.mesh;
    bool hasTexcoords = mesh.texcoords.size() == mesh.vertices.size();
    static std::vector<vec4> clipCoords;
    clipCoords.resize(object.mesh.vertices.size());

    static std::vector<vec3> transformedNormals;
    transformedNormals.resize(object.mesh.vertices.size());
    for (std::size_t instance_id = 0; instance_id < object.modelMatrices.size(); instance_id++) {
        clipCoords.clear();
        transformedNormals.clear();
        VertexShaderUniforms uniforms{
            .VP = VP, .M = object.modelMatrices, .inverseTransposeM = object.inverseTransposeModelMatrices};
#pragma omp parallel for
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            VertexShaderOutput output = vertexShader(
                VertexShaderInput{.position = mesh.vertices[i], .normal = mesh.normals[i], .instance_id = instance_id},
                uniforms);
            clipCoords[i] = std::move(output.position);
            transformedNormals[i] = std::move(output.normal);
        }
#pragma omp parallel for
        for (size_t tri_idx = 0; tri_idx < mesh.indices.size() / 3; tri_idx++) {
            if (clipCoords[mesh.indices[3 * tri_idx]].w < zNear ||
                clipCoords[mesh.indices[3 * tri_idx + 1]].w < zNear ||
                clipCoords[mesh.indices[3 * tri_idx + 2]].w < zNear)
                continue;
            ScreenVertexAttributes attributes[3];
            for (int i = 0; i < 3; i++) {
                const auto& v = clipCoords[mesh.indices[3 * tri_idx + i]];
                float invW = 1.0f / v.w;
                attributes[i].position.x = (v.x * invW + 1) * 0.5f * buffer.width;
                attributes[i].position.y = (v.y * invW + 1) * 0.5f * buffer.height;
                attributes[i].depth = v.z * invW;
                attributes[i].invW = invW;
                if (hasTexcoords) attributes[i].texcoord = mesh.texcoords[mesh.indices[3 * tri_idx + i]];
                attributes[i].normal = transformedNormals[mesh.indices[3 * tri_idx + i]];
            }
            float area = edgeFunction(attributes[0].position, attributes[1].position, attributes[2].position);
            if (area < 1e-6) continue; /* degenerate triangle or not visible */

            drawTriangle(buffer, attributes[0], attributes[1], attributes[2], area, lighting, object.material,
                         hasTexcoords);
        }
    }
}

void Renderer::drawScene(FrameBuffer& buffer, const Scene& scene) const {
    mat4 VP = scene.camera.getPerspectiveMatrix() * scene.camera.getLookAtMatrix();
    for (const auto& object : scene.objects) {
        drawObject(buffer, object, VP, scene.lighting, scene.camera.zNear);
    }
}