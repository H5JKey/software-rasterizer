module;
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <numbers>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>

module app;

import math.vectors;
import math.transforms;
import scene.scene;
import scene.camera;
import model.object;
import model.mesh;
import model.material;
import model.texture;
import renderer.framebuffer;
import renderer.rasterizer;

namespace {

Camera makeCamera(int width, int height) {
    constexpr float pi = std::numbers::pi_v<float>;
    const vec3 eye(0.0f, 1.0f, 3.5f);
    const vec3 target(0.0f, 0.0f, 0.0f);
    const vec3 up(0.0f, 1.0f, 0.0f);
    return Camera(eye, target, up, 90.0f * pi / 180.0f, static_cast<float>(width) / static_cast<float>(height), 0.01f,
                  100.0f);
}

Scene::Lighting makeLighting() {
    Scene::Lighting lighting;
    lighting.ambientColor = Color(0.15, 0.15, 0.15, 1);
    lighting.light.color = Color(1, 1, 1, 1);
    lighting.light.direction = normalize(vec3(1, 0, 0));
    return lighting;
}

}  // namespace

void App::handleEvents(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

App::App(std::size_t w, std::size_t h, const std::string& title)
    : width(static_cast<int>(w)),
      height(static_cast<int>(h)),
      framebuffer(0, 0),
      scene(makeCamera(width, height), makeLighting()) {
    glfwSetErrorCallback(
        [](int error, const char* description) { std::println(stderr, "GLFW error {}: {}", error, description); });

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &this->width, &this->height);
    framebuffer.resize(this->width, this->height);
    glfwSetKeyCallback(window, handleEvents);
    glfwSwapInterval(0);

    buildScene();
}

void App::buildScene() {
    loadedTextures.emplace_back(Texture::loadFromFile("resources/textures/porcelain.jpg"));
    Material material{.baseColor = Color(1, 1, 1, 1), .texture = &loadedTextures[0]};

    scene.objects.emplace_back(Mesh::loadFromFile("resources/Bunny.obj"), std::move(material));
    scene.objects[0].addInstance();

    scene.objects.emplace_back(Mesh::loadFromFile("resources/Chicken_01.obj"),
                               Material{.baseColor = Color(1, 1, 1, 1)});
    for (int i = 0; i < 8; ++i) {
        scene.objects[1].addInstance();
    }
}

void App::run() {
    constexpr float pi = std::numbers::pi_v<float>;

    double report_time = glfwGetTime();
    int report_frames = 0;
    double raster_time = 0.0;
    double output_time = 0.0;

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const float angle = static_cast<float>(now) * 0.5f;

        scene.objects[0].updateTransform(0, rotationY(angle) * scale(vec3(2, 2, 2)));

        const auto instances = scene.objects[1].instancesCount();
        for (std::size_t instance = 0; instance < instances; ++instance) {
            const float theta = angle + (2.0f * pi * static_cast<float>(instance)) / static_cast<float>(instances);
            scene.objects[1].updateTransform(
                instance, rotationZ(theta) * scale(vec3(1, 1, 1)) * translation(vec3(0, 4, 0)) * rotationY(pi / 2));
        }

        glfwGetFramebufferSize(window, &width, &height);
        scene.camera.aspect = static_cast<float>(width) / static_cast<float>(height);
        framebuffer.resize(width, height);

        double start = glfwGetTime();
        framebuffer.clear();
        rasterizer.drawScene(framebuffer, scene);
        raster_time += glfwGetTime() - start;
        start = glfwGetTime();
        glDrawPixels(width, height, GL_RGBA, GL_FLOAT, framebuffer.pixels.data());
        glfwSwapBuffers(window);
        glfwPollEvents();
        output_time += glfwGetTime() - start;

        ++report_frames;
        const double elapsed = glfwGetTime() - report_time;

        if (elapsed >= 1.0) {
            std::printf("frame time: %.3f ms (%.1f FPS) | output: %.3f | raster: %.3f\n",
                        1000.0 * elapsed / report_frames, report_frames / elapsed, output_time, raster_time);
            std::fflush(stdout);

            report_time += elapsed;
            report_frames = 0;
            raster_time = 0.0;
            output_time = 0.0;
        }
    }
}

App::~App() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}