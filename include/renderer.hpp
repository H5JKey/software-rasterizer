#pragma once
#include "frame-buffer.hpp"
#include "vectors.hpp"

class Renderer {
   public:
    void drawTriangle(FrameBuffer& buffer, vec2 v0, vec2 v1, vec2 v2, Color v0Color, Color v1Color, Color v2Color);
};