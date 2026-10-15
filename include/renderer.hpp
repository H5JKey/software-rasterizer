#pragma once
#include "frame-buffer.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include "shaders.hpp"

class Renderer {
    struct VertexAttributes {
        vec2 position;
        float depth;
        vec3 normal;
        float invW;
        vec2 texcoord;
    };

    VertexShader vertexShader;
    FragmentShader fragmentShader;

    void drawTriangle(FrameBuffer& buffer, const VertexAttributes& v0, const VertexAttributes& v1,
                      const VertexAttributes& v2, float area, const Scene::Lighting& lighting,
                      const Material& material) const;

    void drawMesh(FrameBuffer& buffer, const Mesh& mesh, const mat4& M, const mat4& VP, float zNear,
                  const Scene::Lighting& lighting, const Material& material) const;

   public:
    Renderer(VertexShader vertexShader, FragmentShader fragmentShader)
        : vertexShader(std::move(vertexShader)), fragmentShader(std::move(fragmentShader)) {}
    void setFragmentShader(FragmentShader fragmentShader) { this->fragmentShader = std::move(fragmentShader); }
    void drawScene(FrameBuffer& buffer, const Scene& scene) const;
};