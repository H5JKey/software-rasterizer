module;

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <string>
#include <vector>

export module app;
import renderer.framebuffer;
import renderer.rasterizer;
import scene.scene;
import scene.camera;
import model.mesh;
import model.material;
import model.texture;

export class App {
    static constexpr float leftPanelWidth = 150.0f;
    static constexpr float rightPanelWidth = 150.0f;
    static constexpr float bottomPanelHeight = 33.0f;
    static constexpr float topPanelHeight = 33.0f;
    bool showPanels = true;

    GLFWwindow* window;
    int width, height;
    Framebuffer framebuffer;
    Scene scene;
    Rasterizer rasterizer;
    std::vector<Texture> loadedTextures;
    std::array<float, 512> fpsHistory;
    std::size_t frame = 0;

    static void handleEvents(GLFWwindow* window, int key, int, int action, int);
    void buildScene();

    void drawUI();
    void update(double elapsedTime);
    void drawFrame();

   public:
    App(std::size_t width, std::size_t height, const std::string& title);
    void run();
    ~App();
};