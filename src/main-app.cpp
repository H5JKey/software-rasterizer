#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

#include "camera.hpp"
#include "frame-buffer.hpp"
#include "math/transforms.hpp"
#include "math/vectors.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "scene.hpp"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

enum class VisualizationMode { Final = 1, DepthBuffer = 2, Normals = 3, Texcoords = 4 };
VisualizationMode visualizationMode = VisualizationMode::Final;

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/
) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) visualizationMode = VisualizationMode::Final;

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) visualizationMode = VisualizationMode::DepthBuffer;

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) visualizationMode = VisualizationMode::Normals;

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) visualizationMode = VisualizationMode::Texcoords;
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Software Renderer", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return 1;
    }

    int fb_width = 0;
    int fb_height = 0;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    FrameBuffer buffer(fb_width, fb_height);
    Renderer renderer;

    Resources resources;

    resources.loadTexture("../resources/textures/porcelain.jpg", "Porcelain");
    resources.loadTexture("../resources/textures/Grass_13-256x256.png", "Grass");
    resources.loadMesh("../resources/Bunny.obj", "Bunny");

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(0);

    constexpr float pi = 3.14159265358979323846f;

    const vec3 eye(0.0f, 1.0f, 3.5f);
    const vec3 target(0.0f, 0.0f, 0.0f);
    const vec3 up(0.0f, 1.0f, 0.0f);

    Camera camera(eye, target, up, 50.0f * pi / 180.0f, static_cast<float>(fb_width) / static_cast<float>(fb_height),
                  0.01, 100);

    Scene::Lighting lighting;
    lighting.ambientColor = Color(0.15, 0.15, 0.15, 1);
    lighting.light.color = Color(1, 1, 1, 1);
    lighting.light.direction = normalize(vec3(1, 0, 0));
    Scene scene(camera, lighting);

    resources.materials["Grass"] = Material{.texture = &resources.textures["Grass"]};
    resources.materials["Porcelain"] = Material{.texture = &resources.textures["Porcelain"]};

    scene.objects.emplace_back(resources.meshes["Bunny"], resources.materials["Porcelain"]);
    scene.objects.emplace_back(resources.meshes["Bunny"], resources.materials["Grass"]);

    double prev_time = glfwGetTime();
    double report_time = prev_time;
    int report_frames = 0;

    double raster_time = 0.0;
    double output_time = 0.0;

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        prev_time = now;

        const float angle = static_cast<float>(now) * 0.5f;

        scene.objects[0].updateTransform(rotationY(angle) * translation(vec3(-1, 0, 0)) * scale(vec3(0.5, 0.5, 0.5)));
        scene.objects[1].updateTransform(rotationY(-2 * angle));

        double start = glfwGetTime();
        buffer.clear();
        renderer.drawScene(buffer, scene);

        raster_time += glfwGetTime() - start;

        start = glfwGetTime();

        if (visualizationMode == VisualizationMode::Final)
            glDrawPixels(fb_width, fb_height, GL_RGBA, GL_FLOAT, buffer.pixels.data());
        else if (visualizationMode == VisualizationMode::DepthBuffer) {
            std::vector<Color> output(buffer.depth.size());

            float maxDepth = -std::numeric_limits<float>::infinity();
            float minDepth = std::numeric_limits<float>::infinity();
            for (float depth : buffer.depth) {
                if (std::isfinite(depth)) {
                    if (depth > maxDepth) maxDepth = depth;
                    if (depth < minDepth) minDepth = depth;
                }
            }

            float denumerator = 1.0f / (maxDepth - minDepth);
            std::transform(buffer.depth.begin(), buffer.depth.end(), output.begin(),
                           [denumerator, minDepth](float depth) {
                               if (!std::isfinite(depth)) return Color(0, 0, 0, 1);
                               float gray = (depth - minDepth) * denumerator;
                               return Color(gray, gray, gray, 1.0f);
                           });
            glDrawPixels(fb_width, fb_height, GL_RGBA, GL_FLOAT, output.data());
        } else if (visualizationMode == VisualizationMode::Normals)
            glDrawPixels(fb_width, fb_height, GL_RGBA, GL_FLOAT, buffer.normals.data());
        else if (visualizationMode == VisualizationMode::Texcoords) {
            std::vector<Color> output(buffer.depth.size());

            std::transform(buffer.texcoords.begin(), buffer.texcoords.end(), output.begin(),
                           [](vec2 uv) { return Color(uv.u, uv.v, 0.0f, 1.0f); });
            glDrawPixels(fb_width, fb_height, GL_RGBA, GL_FLOAT, output.data());
        }
        glfwSwapBuffers(window);
        glfwPollEvents();

        output_time += glfwGetTime() - start;

        ++report_frames;

        const double elapsed = glfwGetTime() - report_time;

        if (elapsed >= 1.0) {
            printf(
                "frame time: %.3f ms (%.1f FPS) | "
                "output: %.3f | raster: %.3f\n",
                1000.0 * elapsed / report_frames, report_frames / elapsed, output_time, raster_time);
            fflush(stdout);

            report_time += elapsed;
            report_frames = 0;
            raster_time = 0.0;
            output_time = 0.0;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}