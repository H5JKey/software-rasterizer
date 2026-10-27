module;

#include <algorithm>
#include <cstddef>
#include <vector>
#include <utility>

export module core.engine.renderer;

import core.frameBuffer;
import math.vectors;
import math.matrices;
import core.scene;
import core.engine.shaders;

export class Renderer {
    std::vector<vec3> transformedNormals;
    std::vector<vec4> clipCoords;
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
                    float zNear);

    void normalizeDepth(FrameBuffer& buffer);

   public:
    enum class RenderMode {
        Shaded = 0,
        Unlit = 1,
        Depth = 2,
        Normals = 3,
        Texcoords = 4,
    };
    RenderMode mode = RenderMode::Shaded;

    Renderer() : vertexShader(defaultShaders::vertexShader), fragmentShader(defaultShaders::lambertShader) {
        transformedNormals.reserve(1024);
        clipCoords.reserve(1024);
    }
    void setFragmentShader(FragmentShader fragmentShader) { this->fragmentShader = std::move(fragmentShader); }
    void drawScene(FrameBuffer& buffer, const Scene& scene);

    void setRenderMode(RenderMode mode);
};