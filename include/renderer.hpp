#pragma once
#include "frame-buffer.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "mesh.hpp"
#include "scene.hpp"

class Renderer {
    struct VertexAttributes {
        vec2 position;
        float depth;
        Color color;
        vec3 normal;
    };

    void drawTriangle(FrameBuffer& buffer, const VertexAttributes& v0, const VertexAttributes& v1,
                      const VertexAttributes& v2, float area, const Scene::Lighting& lighting) const;

    void drawMesh(FrameBuffer& buffer, const Mesh& mesh, const mat4& M, const mat4& VP, float zNear,
                  const Scene::Lighting& lighting) const;

   public:
    void drawScene(FrameBuffer& buffer, const Scene& scene) const;
};