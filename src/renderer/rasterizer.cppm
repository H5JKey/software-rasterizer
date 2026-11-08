module;

#include <cstddef>
#include <vector>

export module renderer.rasterizer;

import renderer.framebuffer;
import math;
import scene.scene;
import renderer.shaders;
import model.object;
import model.material;
import model.mesh;
import scene.skybox;
import scene.terrain;

export class Rasterizer {
    std::vector<vec3> transformedNormals;
    std::vector<vec3> transformedWorldPositions;
    std::vector<vec4> clipCoords;
    std::vector<vec2> transformedTexcoords;
    struct ScreenVertexAttributes {
        vec2 position;
        float depth;
        vec3 normal;
        vec3 worldPosition;
        float invW;
        vec2 texcoord;
    };

    std::size_t passIndex = 0;

    const shaders::Pipeline& currentPipeline() const { return shaders::passes[passIndex]; }

    void drawTriangle(Framebuffer& buffer, const ScreenVertexAttributes& v0, const ScreenVertexAttributes& v1,
                      const ScreenVertexAttributes& v2, float area, const shaders::FrameUniforms& frameUniforms,
                      const shaders::DrawUniforms& drawUniforms) const;

    void drawObject(Framebuffer& buffer, const Object& object, const shaders::FrameUniforms& frameUniforms, float zNear,
                    const Terrain* terrain = nullptr);

    void runScreenStages(Framebuffer& buffer, const shaders::FrameUniforms& frameUniforms);

   public:
    Rasterizer() {
        transformedNormals.reserve(1024);
        transformedWorldPositions.reserve(1024);
        clipCoords.reserve(1024);
        transformedTexcoords.reserve(1024);
    }

    void drawScene(Framebuffer& buffer, const Scene& scene);

    std::size_t getPassIndex() const { return passIndex; }

    void setPass(std::size_t index) { passIndex = index < shaders::passesCount ? index : 0; }
};
