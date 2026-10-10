#pragma once
#include "camera.hpp"
#include "frame-buffer.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "mesh.hpp"
#include "object.hpp"

class Renderer {
    struct VertexAttributes {
        vec2 position;
        float depth;
        Color color;
    };

   public:
    void drawTriangle(FrameBuffer& buffer, const VertexAttributes& v0, const VertexAttributes& v1,
                      const VertexAttributes& v2, float area) const;

    void drawMesh(FrameBuffer& buffer, const Mesh& mesh, const mat4& MVP, float zNear) const;

   public:
    void drawObjects(FrameBuffer& buffer, const std::vector<Object>& objects, const Camera& camera) const;
};