#include "renderer.hpp"

#include <cmath>

void Renderer::drawTriangle(FrameBuffer& buffer, vec2 v0, vec2 v1, vec2 v2, Color v0Color, Color v1Color,
                            Color v2Color) {
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

            buffer.pixels[py * buffer.width + px] = v0Color;
        }
    }
}