module;

#include <utility>
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

export class Rasterizer {
    std::vector<vec3> transformedNormals;
    std::vector<vec4> clipCoords;
    struct ScreenVertexAttributes {
        vec2 position;
        float depth;
        vec3 normal;
        float invW;
        vec2 texcoord;
    };

    shaders::FragmentShader fragmentShader;

    void drawTriangle(Framebuffer& buffer, const ScreenVertexAttributes& v0, const ScreenVertexAttributes& v1,
                      const ScreenVertexAttributes& v2, float area, const Scene::Lighting& lighting,
                      const Material& material, bool hasTexcoords = true) const;

    void drawObject(Framebuffer& buffer, const Object& object, const mat4& VP, const Scene::Lighting& lighting,
                    float zNear);

    void normalizeDepth(Framebuffer& buffer);

    void drawSkybox(Framebuffer& buffer, const mat4& VP, const Skybox& skybox, const vec3& cameraPosition);

   public:
    enum class RenderMode {
        Shaded = 0,
        Unlit = 1,
        Depth = 2,
        Normals = 3,
        Texcoords = 4,
    };
    RenderMode mode = RenderMode::Shaded;

    Rasterizer() : fragmentShader(shaders::lambertShader) {
        transformedNormals.reserve(1024);
        clipCoords.reserve(1024);
    }
    void drawScene(Framebuffer& buffer, const Scene& scene);

    void setRenderMode(RenderMode mode);
};