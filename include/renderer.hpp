#pragma once
#include "frame-buffer.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "mesh.hpp"

class Renderer {
   public:
    void drawTriangle(FrameBuffer& buffer, vec2 v0, vec2 v1, vec2 v2, Color v0Color, Color v1Color,
                      Color v2Color) const;

    void drawMesh(FrameBuffer& buffer, const Mesh& mesh, const mat4& MVP) const;
};