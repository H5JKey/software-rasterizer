#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "camera.hpp"
#include "frame-buffer.hpp"
#include "math/transforms.hpp"
#include "mesh-utils.hpp"
#include "renderer.hpp"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/
) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
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

    Mesh m = load_obj("../resources/Bunny.obj");
    std::vector<Object> objects;
    objects.emplace_back(m);
    objects.emplace_back(m);

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(0);

    constexpr float pi = 3.14159265358979323846f;

    const vec3 eye(0.0f, 1.0f, 3.5f);
    const vec3 target(0.0f, 0.0f, 0.0f);
    const vec3 up(0.0f, 1.0f, 0.0f);

    Camera camera(eye, target, up, 75.0f * pi / 180.0f, static_cast<float>(fb_width) / static_cast<float>(fb_height),
                  0.01, 100);
    double prev_time = glfwGetTime();
    double report_time = prev_time;
    int report_frames = 0;

    double raster_time = 0.0;
    double output_time = 0.0;

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        prev_time = now;

        const float angle = static_cast<float>(now) * 0.8f;

        objects[0].updateTransform(translation(vec3(-1, 0, 0)) * rotationY(angle) * scale(vec3(0.5, 0.5, 0.5)));
        objects[1].updateTransform(translation(vec3(1, 0, 0)) * rotationY(-2 * angle) * scale(vec3(0.8, 0.8, 0.8)));

        double start = glfwGetTime();
        buffer.clear();
        renderer.drawObjects(buffer, objects, camera);

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