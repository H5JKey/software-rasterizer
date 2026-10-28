module;

#include <GL/gl.h>
#include <GLFW/glfw3.h>

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
    GLFWwindow* window;
    int width, height;
    Framebuffer framebuffer;
    Scene scene;
    Rasterizer rasterizer;
    std::vector<Texture> loadedTextures;

    static void handleEvents(GLFWwindow* window, int key, int, int action, int);
    void buildScene();

   public:
    App(std::size_t width, std::size_t height, const std::string& title);
    void run();
    ~App();
};