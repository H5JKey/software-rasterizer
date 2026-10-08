// Пример вывода массива пикселей в окно в реальном времени.
//
// Каждый кадр программа на CPU заполняет буфер RGBA8 цветом, который плавно
// меняется со временем, и выводит его в окно через glDrawPixels. OpenGL здесь
// используется только для того, чтобы показать готовый массив пикселей.
// Раз в секунду в консоль печатается среднее время кадра.

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <print>

#include "frame-buffer.hpp"
#include "renderer.hpp"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(400, 300, "Software Renderer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    int fb_width = 0, fb_height = 0;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    FrameBuffer buffer(fb_width, fb_height);
    Renderer renderer;

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    // 0 - не ждать вертикальной синхронизации: время кадра показывает реальную
    // скорость программы, а не частоту монитора. 1 - включить vsync.
    glfwSwapInterval(0);

    // Число пикселей окна может отличаться от width x height (масштабирование
    // экрана в ОС), поэтому размер буфера берём у самого окна.

    // RGBA8, строки снизу вверх: первая строка буфера - нижняя строка окна

    double prev_time = glfwGetTime();
    double report_time = prev_time;
    int report_frames = 0;

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const float dt = (float)(now - prev_time);  // длительность прошлого кадра, с
        prev_time = now;

        renderer.drawTriangle(buffer, {0, 0}, {0, 100}, {100, 100}, {255, 255, 255, 255}, {255, 255, 255, 255},
                              {255, 255, 255, 255});

        glDrawPixels(fb_width, fb_height, GL_RGBA, GL_FLOAT, buffer.pixels.data());
        glfwSwapBuffers(window);
        glfwPollEvents();

        report_frames++;
        const double elapsed = glfwGetTime() - report_time;
        if (elapsed >= 1.0) {
            printf("frame time: %.3f ms (%.1f FPS)\n", 1000.0 * elapsed / report_frames, report_frames / elapsed);
            fflush(stdout);
            report_time += elapsed;
            report_frames = 0;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
