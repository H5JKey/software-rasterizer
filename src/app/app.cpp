module;
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <format>
#include <numbers>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>

module app;

import math;
import scene.scene;
import scene.camera;
import model.object;
import model.mesh;
import model.material;
import model.texture;
import renderer.framebuffer;
import renderer.rasterizer;
import app.theme;

namespace {

Camera makeCamera(int width, int height) {
    constexpr float pi = std::numbers::pi_v<float>;
    const vec3 eye(0.0f, 1.0f, 3.5f);
    return Camera(eye, 0.0f, 0.0f, 90.0f * pi / 180.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f,
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

void App::handleKeyboard(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard && action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            app->showPanels = !app->showPanels;
            glfwSetInputMode(window, GLFW_CURSOR, app->showPanels ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            app->mouseInfo.firstMouse = true;
        } else if (key == GLFW_KEY_EQUAL) {
            app->scaleFactor += 0.1f;
            if (app->scaleFactor > 3.f) {
                app->scaleFactor = 3.0f;
                return;
            }
            app->leftPanelWidth = app->baseLeftPanelWidth * app->scaleFactor;
            app->rightPanelWidth = app->baseRightPanelWidth * app->scaleFactor;
            app->bottomPanelHeight = app->baseBottomPanelHeight * app->scaleFactor;
            app->topPanelHeight = app->baseTopPanelHeight * app->scaleFactor;
        } else if (key == GLFW_KEY_MINUS) {
            app->scaleFactor -= 0.1f;
            if (app->scaleFactor < 0.5f) {
                app->scaleFactor = 0.5f;
                return;
            }
            app->leftPanelWidth = app->baseLeftPanelWidth * app->scaleFactor;
            app->rightPanelWidth = app->baseRightPanelWidth * app->scaleFactor;
            app->bottomPanelHeight = app->baseBottomPanelHeight * app->scaleFactor;
            app->topPanelHeight = app->baseTopPanelHeight * app->scaleFactor;
        } else if (!app->showPanels) {
            if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D) {
                const float speed = 0.1f;
                if (key == GLFW_KEY_W) {
                    app->scene.camera.velocity += vec3(0, 0, -speed);
                } else if (key == GLFW_KEY_S) {
                    app->scene.camera.velocity += vec3(0, 0, speed);
                } else if (key == GLFW_KEY_A) {
                    app->scene.camera.velocity += vec3(-speed, 0, 0);
                } else if (key == GLFW_KEY_D) {
                    app->scene.camera.velocity += vec3(speed, 0, 0);
                }
            }
        }
    }
}

void App::handleCursor(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || app->showPanels) return;

    if (app->mouseInfo.firstMouse) {
        app->mouseInfo.prevMousePosX = xpos;
        app->mouseInfo.prevMousePosY = ypos;
        app->mouseInfo.firstMouse = false;
        return;
    }
    const float dx = static_cast<float>(xpos - app->mouseInfo.prevMousePosX);
    const float dy = static_cast<float>(ypos - app->mouseInfo.prevMousePosY);
    app->mouseInfo.prevMousePosX = xpos;
    app->mouseInfo.prevMousePosY = ypos;

    app->scene.camera.yaw -= app->mouseInfo.sensetive * dx;
    app->scene.camera.pitch -= app->mouseInfo.sensetive * dy;

    const float limit = std::numbers::pi_v<float> / 2.0f - 0.01f;
    app->scene.camera.pitch = std::clamp(app->scene.camera.pitch, -limit, limit);
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
    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &this->width, &this->height);
    framebuffer.resize(this->width, this->height);
    glfwSwapInterval(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    Theme::applyDarkTheme();

    glfwSetKeyCallback(window, handleKeyboard);
    glfwSetCursorPosCallback(window, handleCursor);
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(nullptr);
    buildScene();
}

void App::buildScene() {
    loadedTextures.emplace_back(Texture::loadFromFile("resources/textures/porcelain.jpg"));
    Material material{.baseColor = Color(1, 1, 1, 1), .texture = &loadedTextures[0]};

    scene.addObject(Object{Mesh::loadFromFile("resources/Bunny.obj"), std::move(material)});
    scene.addObjectInstance(0);
    scene.updateObjectTransform(0, 0, scale(2, 2, 2));
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
        frame++;
    }
}

void App::update(double elapsedTime) {
    int viewPortWidth = width;
    int viewPortHeight = height;
    if (showPanels) {
        viewPortHeight -= (static_cast<int>(topPanelHeight) + static_cast<int>(bottomPanelHeight));
        viewPortWidth -= (static_cast<int>(leftPanelWidth) + static_cast<int>(rightPanelWidth));
    }
    if (framebuffer.width != viewPortWidth || framebuffer.height != viewPortHeight) {
        framebuffer.resize(viewPortWidth, viewPortHeight);
        scene.camera.aspect = static_cast<float>(framebuffer.width) / static_cast<float>(framebuffer.height);
    }
    scene.camera.update(elapsedTime);
}

void App::drawFrame() {
    framebuffer.clear();
    rasterizer.drawScene(framebuffer, scene);

    const int vx = static_cast<int>(showPanels ? leftPanelWidth : 0);
    const int vy = static_cast<int>(showPanels ? topPanelHeight : 0);
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

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    style.FontScaleMain = 1.4 * scaleFactor;

    const float W = io.DisplaySize.x;
    const float H = io.DisplaySize.y;
    if (showPanels) {
        ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

        if (leftPanelWidth > 0.0) {
            ImGui::SetNextWindowPos(ImVec2(0, topPanelHeight));
            ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, H - topPanelHeight));
            ImGui::Begin("##left", nullptr, panelFlags);
            ImGui::SeparatorText("Camera");
            ImGui::SliderFloat("Fov", &scene.camera.fov, 0.05f, 3.14f, "%.2f");
            ImGui::SliderFloat("ZNear", &scene.camera.zNear, 0.1, 20.0, "%.1f");
            ImGui::SliderFloat("ZFar", &scene.camera.zFar, 20.0, 100.0f, "%.1f");
            ImGui::End();
        }
        if (rightPanelWidth > 0.0) {
            ImGui::SetNextWindowPos(ImVec2(W - rightPanelWidth, topPanelHeight));
            ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, H - topPanelHeight));
            ImGui::Begin("##right", nullptr, panelFlags);
            ImGui::SeparatorText("Performance");
            ImGui::Text("%.1f FPS | %.2f ms", io.Framerate, 1000 / io.Framerate);
            ImGui::PlotLines("##FPSHistory", fpsHistory.data(), fpsHistory.size(), frame % fpsHistory.size(), nullptr,
                             0.0f, 240.f, ImVec2(ImGui::GetContentRegionAvail().x, 80));

            ImGui::SeparatorText("Scene Info");

            ImGui::Text("Objects: %zu", scene.stats.objectsCount);
            ImGui::Text("Instances: %zu", scene.stats.instancesCount);
            ImGui::Text("Triangles: %zu", scene.stats.totalTrianglesCount);

            ImGui::End();
        }
        if (topPanelHeight > 0.0) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(W, topPanelHeight));
            ImGui::Begin("##top", nullptr, panelFlags);
            ImGui::End();
        }
        if (bottomPanelHeight > 0.0) {
            ImGui::SetNextWindowPos(ImVec2(leftPanelWidth, H - bottomPanelHeight));
            ImGui::SetNextWindowSize(ImVec2(W - leftPanelWidth - rightPanelWidth, bottomPanelHeight));
            ImGui::Begin("##bottom", nullptr, panelFlags);
            ImGui::End();
        }
    }
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    std::string fpsString = std::format("{:.0f} FPS", io.Framerate);
    fpsHistory[frame % fpsHistory.size()] = io.Framerate;
    const ImVec2 textSize = ImGui::CalcTextSize(fpsString.c_str());
    const ImVec2 pos(W - (showPanels ? rightPanelWidth : 0) - 12.0f - textSize.x,
                     (showPanels ? topPanelHeight : 0) + 8.0f);

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