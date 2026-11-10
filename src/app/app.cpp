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
#include <filesystem>
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
import renderer.viewport;
import renderer.rasterizer;
import renderer.shaders;
import scene.skybox;
import scene.terrain;
import app.theme;

namespace {

Camera makeCamera(int width, int height, vec3 eye = vec3(0, 0, 0)) {
    constexpr float pi = std::numbers::pi_v<float>;
    return Camera(std::move(eye), 0.0f, 0.0f, 90.0f * pi / 180.0f,
                  static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
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

App::App(int vw, int vh, int rw, int rh, const std::string& title,
         const std::optional<std::filesystem::path>& savedHeightMapFilename)
    : width(vw),
      height(vh),
      renderWidth(rw),
      renderHeight(rh),
      framebuffer(0, 0),
      viewport(0, 0),
      scene(makeCamera(width, height, vec3(0, 2, 0)), makeLighting(),
            Skybox("resources/textures/skybox/hills_lf.png", "resources/textures/skybox/hills_rt.png",
                   "resources/textures/skybox/hills_ft.png", "resources/textures/skybox/hills_bk.png",
                   "resources/textures/skybox/hills_up.jpg", "resources/textures/skybox/hills_dn.png"),
            Terrain(42, "resources/textures/Grass_09-256x256.png", "resources/textures/Grass_11-256x256.png")),
      savedCamera(makeCamera(width, height, vec3(0, scene.terrain.heightScale, 0))) {
    glfwSetErrorCallback(
        [](int error, const char* description) { std::println(stderr, "GLFW error {}: {}", error, description); });

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    if (savedHeightMapFilename) scene.terrain.getHeightMap().saveToPng(256, 256, *savedHeightMapFilename);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &this->width, &this->height);
    framebuffer.resize(renderWidth, renderHeight);
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
    Material material{
        .baseColor = Color(1, 1, 1, 1),
        .textures = {&TextureWithMipLevels::loadFromFile("resources/textures/porcelain.jpg")},
        .shader = shaders::defaultMaterialShader,
    };

    scene.addObject(Object{Mesh::loadFromFile("resources/Bunny.obj"), std::move(material)});
    scene.addObjectInstance(0);
    scene.updateObjectTransform(0, 0, translation(0, 5, 0) * scale(2, 2, 2));
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
    renderWidth = std::min(renderWidth, width);
    renderHeight = std::min(renderHeight, height);
    if (showPanels) {
        viewPortHeight -= (static_cast<int>(topPanelHeight) + static_cast<int>(bottomPanelHeight));
        viewPortWidth -= (static_cast<int>(leftPanelWidth) + static_cast<int>(rightPanelWidth));
    }
    if (viewport.width != viewPortWidth || viewport.height != viewPortHeight) {
        viewport.resize(viewPortWidth, viewPortHeight);
        scene.camera.aspect = static_cast<float>(viewport.width) / static_cast<float>(viewport.height);
    }
    if (framebuffer.width != renderWidth || framebuffer.height != renderHeight) {
        framebuffer.resize(renderWidth, renderHeight);
    }
    if (followTerrain) {
        scene.camera.yaw = 0;
        scene.camera.pitch = 0;
        scene.camera.velocity += vec3(0, 0, -1);
    }

    scene.update(elapsedTime);
}

void App::drawFrame() {
    framebuffer.clear();
    rasterizer.drawScene(framebuffer, scene);
    viewport.upscaleFromFramebuffer(framebuffer, currentUpscalerFilter);

    const int vx = static_cast<int>(showPanels ? leftPanelWidth : 0);
    const int vy = static_cast<int>(showPanels ? topPanelHeight : 0);
    const int vw = viewport.width;
    const int vh = viewport.height;
    if (vw <= 0 || vh <= 0) return;

    const int glY = height - vy - vh;

    glViewport(vx, glY, vw, vh);
    glEnable(GL_SCISSOR_TEST);
    glScissor(vx, glY, vw, vh);

    glRasterPos2i(-1, -1);
    glDrawPixels(vw, vh, GL_RGBA, GL_FLOAT, viewport.pixels.data());

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
            ImGui::SeparatorText("Modes");

            int modeInt = static_cast<int>(rasterizer.getPassIndex());
            bool changed = false;
            for (int i = 0; i < static_cast<int>(shaders::passesCount); ++i) {
                changed |= ImGui::RadioButton(shaders::passes[i].name.data(), &modeInt, i);
            }
            if (changed) {
                rasterizer.setPass(static_cast<std::size_t>(modeInt));
            }

            ImGui::SeparatorText("Camera");
            ImGui::SliderFloat("Fov", &scene.camera.fov, 0.05f, 3.14f, "%.2f");
            ImGui::SliderFloat("ZNear", &scene.camera.zNear, 0.1, 20.0, "%.1f");
            ImGui::SliderFloat("ZFar", &scene.camera.zFar, 20.0, 100.0f, "%.1f");
            if (ImGui::Checkbox("Follow terrain", &followTerrain)) {
                std::swap(savedCamera, scene.camera);
            }

            ImGui::SeparatorText("Render");
            ImGui::DragInt("Width##render", &renderWidth, 1.0f, 16, width, "%d", ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragInt("Height##render", &renderHeight, 1.0f, 16, height, "%d", ImGuiSliderFlags_AlwaysClamp);

            ImGui::Text("Filter");
            int filter = static_cast<int>(currentUpscalerFilter);
            changed = false;
            changed |= ImGui::RadioButton("Nearest", &filter, 0);
            changed |= ImGui::RadioButton("Bilinear", &filter, 1);
            if (changed) currentUpscalerFilter = static_cast<Filter>(filter);

            ImGui::Text("Framebuffer: %dx%d", framebuffer.width, framebuffer.height);
            ImGui::Text("Viewport: %dx%d", viewport.width, viewport.height);
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