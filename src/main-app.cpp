#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <limits>
#include <numbers>

#include "camera.hpp"
#include "frame-buffer.hpp"
#include "math/transforms.hpp"
#include "math/vectors.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "shaders.hpp"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

Renderer renderer;

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/
) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_0 && action == GLFW_PRESS) renderer.setRenderMode(Renderer::RenderMode::Shaded);

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) renderer.setRenderMode(Renderer::RenderMode::Unlit);

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) renderer.setRenderMode(Renderer::RenderMode::Normals);

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) renderer.setRenderMode(Renderer::RenderMode::Depth);

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) renderer.setRenderMode(Renderer::RenderMode::Texcoords);
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

    Texture texture = Texture::loadFromFile("../resources/textures/porcelain.jpg");
    Material material{.baseColor = Color(1, 1, 1, 1), .texture = &texture};

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(0);

    constexpr float pi = std::numbers::pi;

    const vec3 eye(0.0f, 1.0f, 3.5f);
    const vec3 target(0.0f, 0.0f, 0.0f);
    const vec3 up(0.0f, 1.0f, 0.0f);

    Camera camera(eye, target, up, 90.0f * pi / 180.0f, static_cast<float>(fb_width) / static_cast<float>(fb_height),
                  0.01, 100);

    Scene::Lighting lighting;
    lighting.ambientColor = Color(0.15, 0.15, 0.15, 1);
    lighting.light.color = Color(1, 1, 1, 1);
    lighting.light.direction = normalize(vec3(1, 0, 0));
    Scene scene(camera, lighting);

    scene.objects.emplace_back(Mesh::loadFromFile("../resources/Bunny.obj"), material);
    scene.objects[0].addInstance();

    scene.objects.emplace_back(Mesh::loadFromFile("../resources/Chicken_01.obj"),
                               Material{.baseColor = Color(1, 1, 1, 1)});
    for (int i = 0; i < 8; i++) {
        scene.objects[1].addInstance();
    }

    double prev_time = glfwGetTime();
    double report_time = prev_time;
    int report_frames = 0;

    double raster_time = 0.0;
    double output_time = 0.0;

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        prev_time = now;

        const float angle = static_cast<float>(now) * 0.5f;

        scene.objects[0].updateTransform(0, rotationY(angle) * scale(vec3(2, 2, 2)));
        for (int instance = 0; instance < 8; instance++) {
            float theta = angle + (2.0f * pi * instance) / scene.objects[1].instancesCount();
            scene.objects[1].updateTransform(
                instance, rotationZ(theta) * scale(vec3(1, 1, 1)) * translation(vec3(0, 4, 0)) * rotationY(pi / 2));
        }

        double start = glfwGetTime();
        buffer.clear();
        renderer.drawScene(buffer, scene);

        raster_time += glfwGetTime() - start;

        start = glfwGetTime();

        glDrawPixels(fb_width, fb_height, GL_RGBA, GL_FLOAT, buffer.pixels.data());

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