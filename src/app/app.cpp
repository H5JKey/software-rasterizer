module;
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cmath>
#include <cstdio>
#include <format>
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
    glfwSwapInterval(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    glfwSetKeyCallback(window, handleEvents);
    ImGui_ImplOpenGL3_Init(nullptr);

    buildScene();
}

void App::buildScene() {
    loadedTextures.emplace_back(Texture::loadFromFile("resources/textures/porcelain.jpg"));
    Material material{.baseColor = Color(1, 1, 1, 1), .texture = &loadedTextures[0]};

    scene.objects.emplace_back(Mesh::loadFromFile("resources/Bunny.obj"), std::move(material));
    scene.objects[0].addInstance();
    scene.objects[0].updateTransform(0, scale(2, 2, 2));
}

void App::run() {
    double elapsedTime = 0;
    double prevTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwGetFramebufferSize(window, &width, &height);

        update(elapsedTime);
        drawUI();
        drawFrame();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        double now = glfwGetTime();
        elapsedTime = now - prevTime;
        prevTime = now;
    }
}

void App::update(double elapsedTime) {
    const int viewPortWidth = width - (int)leftPanelWidth - (int)rightPanelWidth;
    const int viewPortHeight = height - (int)topPanelHeight - (int)bottomPanelHeight;
    scene.camera.aspect = static_cast<float>(viewPortWidth) / static_cast<float>(viewPortHeight);
    if (framebuffer.width != viewPortWidth || framebuffer.height != viewPortHeight)
        framebuffer.resize(viewPortWidth, viewPortHeight);
}

void App::drawFrame() {
    framebuffer.clear();
    rasterizer.drawScene(framebuffer, scene);

    const int vx = static_cast<int>(leftPanelWidth);
    const int vy = static_cast<int>(topPanelHeight);
    const int vw = framebuffer.width;
    const int vh = framebuffer.height;
    if (vw <= 0 || vh <= 0) return;

    const int glY = height - vy - vh;

    glViewport(vx, glY, vw, vh);
    glEnable(GL_SCISSOR_TEST);
    glScissor(vx, glY, vw, vh);

    glRasterPos2i(-1, -1);
    glDrawPixels(vw, vh, GL_RGBA, GL_FLOAT, framebuffer.pixels.data());

    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, width, height);
}

void App::drawUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                  ImGuiWindowFlags_NoScrollbar;

    ImGuiIO& io = ImGui::GetIO();
    const float W = io.DisplaySize.x;
    const float H = io.DisplaySize.y;

    if (topPanelHeight > 0.0) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(W, topPanelHeight));
        ImGui::Begin("##top", nullptr, panelFlags);
        ImGui::End();
    }
    if (leftPanelWidth > 0.0) {
        ImGui::SetNextWindowPos(ImVec2(0, topPanelHeight));
        ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, H - topPanelHeight - bottomPanelHeight));
        ImGui::Begin("##left", nullptr, panelFlags);

        ImGui::End();
    }
    if (rightPanelWidth > 0.0) {
        ImGui::SetNextWindowPos(ImVec2(W - rightPanelWidth, topPanelHeight));
        ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, H - topPanelHeight - bottomPanelHeight));
        ImGui::Begin("##right", nullptr, panelFlags);
        ImGui::SliderFloat("Fov", &scene.camera.fov, 0.01f, 3.14f, "%.2f rad");
        ImGui::End();
    }
    if (bottomPanelHeight > 0.0) {
        ImGui::SetNextWindowPos(ImVec2(0, H - bottomPanelHeight));
        ImGui::SetNextWindowSize(ImVec2(W, bottomPanelHeight));
        ImGui::Begin("##bottom", nullptr, panelFlags);

        ImGui::End();
    }

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    std::string fpsString = std::format("{:.1f} FPS  |  {:.2f} ms", io.Framerate, 1000.0f / io.Framerate);
    const ImVec2 textSize = ImGui::CalcTextSize(fpsString.c_str());
    const ImVec2 pos(W - rightPanelWidth - 12.0f - textSize.x, topPanelHeight + 8.0f);

    dl->AddRectFilled(ImVec2(pos.x - 6, pos.y - 2), ImVec2(pos.x + textSize.x + 6, pos.y + textSize.y + 2),
                      IM_COL32(0, 0, 0, 140), 4.0f);
    dl->AddText(pos, IM_COL32(230, 230, 230, 255), fpsString.c_str());

    ImGui::Render();
}

App::~App() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}