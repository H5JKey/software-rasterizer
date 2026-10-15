#pragma once
#include "frame-buffer.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "scene.hpp"
#include "shaders.hpp"

class Renderer {
    struct ScreenVertexAttributes {
        vec2 position;
        float depth;
        vec3 normal;
        float invW;
        vec2 texcoord;
    };

    VertexShader vertexShader;
    FragmentShader fragmentShader;

    void drawTriangle(FrameBuffer& buffer, const ScreenVertexAttributes& v0, const ScreenVertexAttributes& v1,
                      const ScreenVertexAttributes& v2, float area, const Scene::Lighting& lighting,
                      const Material& material, bool hasTexcoords = true) const;

    void drawObject(FrameBuffer& buffer, const Object& object, const mat4& VP, const Scene::Lighting& lighting,
                    float zNear) const;

   public:
    Renderer(VertexShader vertexShader, FragmentShader fragmentShader)
        : vertexShader(std::move(vertexShader)), fragmentShader(std::move(fragmentShader)) {}
    void setFragmentShader(FragmentShader fragmentShader) { this->fragmentShader = std::move(fragmentShader); }
    void drawScene(FrameBuffer& buffer, const Scene& scene) const;
};