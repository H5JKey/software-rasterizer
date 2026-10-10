#pragma once
#include "camera.hpp"
#include "frame-buffer.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "mesh.hpp"
#include "object.hpp"

class Renderer {
   private:
    void drawTriangle(FrameBuffer& buffer, vec2 v0, vec2 v1, vec2 v2, Color v0Color, Color v1Color,
                      Color v2Color) const;

    void drawMesh(FrameBuffer& buffer, const Mesh& mesh, const mat4& MVP, float zNear) const;

   public:
    void drawObjects(FrameBuffer& buffer, const std::vector<Object>& objects, const Camera& camera) const;
};