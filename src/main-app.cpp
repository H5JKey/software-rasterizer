#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <limits>

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

    VertexShader vertexShader = [](const VertexShaderInput& input,
                                   const VertexShaderUniforms& uniforms) -> VertexShaderOutput {
        VertexShaderOutput output;
        output.position = uniforms.VP * uniforms.M[input.instance_id] *
                          vec4(input.position.x, input.position.y, input.position.z, 1.0);
        output.normal = uniforms.inverseTransposeM[input.instance_id] * input.normal;
        return output;
    };

    FragmentShader fragmentShader = [](const FragmentShaderInput& input,
                                       const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
        FragmentShaderOutput output;
        if (input.hasTexcoords && input.material.texture) {
            output.color = input.material.texture->sample(input.uv);
        } else {
            output.color = input.material.baseColor;
        }
        Color factor =
            (uniforms.lighting.ambientColor +
             uniforms.lighting.light.color * std::max(0.f, dot(input.normal, uniforms.lighting.light.direction)));
        output.color.r = output.color.r * factor.r;
        output.color.g = output.color.g * factor.g;
        output.color.b = output.color.b * factor.b;
        return output;
    };

    FragmentShader normalFragmentShader = [](const FragmentShaderInput& input,
                                             const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
        FragmentShaderOutput output;
        output.color = Color(input.normal.x, input.normal.y, input.normal.z, 1.0);
        return output;
    };

    FragmentShader depthFragmentShader = [](const FragmentShaderInput& input,
                                            const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
        FragmentShaderOutput output;
        constexpr float minDepth = 0.99;
        constexpr float maxDepth = 0.991;
        constexpr float denum = 1 / (maxDepth - minDepth);
        float gray = (input.depth - minDepth) * denum;
        output.color = Color(gray, gray, gray, 1.0f);
        return output;
    };

    FragmentShader texcoordsFragmentShader = [](const FragmentShaderInput& input,
                                                const FragmentShaderUniforms& uniforms) -> FragmentShaderOutput {
        FragmentShaderOutput output;
        if (input.hasTexcoords)
            output.color = Color(input.uv.x, input.uv.y, 0.0f, 1.0f);
        else
            output.color = Color(0, 0, 0, 1);
        return output;
    };

    Renderer renderer(vertexShader, fragmentShader);

    Texture texture = Texture::loadFromFile("../resources/textures/porcelain.jpg");
    Material material{.baseColor = Color(1, 1, 1, 1), .texture = &texture};

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(0);

    constexpr float pi = 3.14159265358979323846f;

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
        if (visualizationMode == VisualizationMode::Final)
            renderer.setFragmentShader(fragmentShader);
        else if (visualizationMode == VisualizationMode::Normals)
            renderer.setFragmentShader(normalFragmentShader);
        else if (visualizationMode == VisualizationMode::Texcoords)
            renderer.setFragmentShader(texcoordsFragmentShader);
        else if (visualizationMode == VisualizationMode::DepthBuffer)
            renderer.setFragmentShader(depthFragmentShader);

        const double now = glfwGetTime();
        prev_time = now;

        const float angle = static_cast<float>(now) * 0.5f;

        scene.objects[0].updateTransform(0, rotationY(angle) * scale(vec3(2, 2, 2)));
        for (int instance = 0; instance < 8; instance++) {
            float theta = angle + (2.0f * pi * instance) / 8.0f;
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