
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <numbers>
#include <optional>
#include <print>
#include <string>

import core.scene;
import core.camera;
import core.engine.renderer;
import core.texture;
import math.vectors;
import math.transforms;
import core.mesh;
import core.material;
import utils.images;
import core.frameBuffer;

void printHelp(const std::string& programName) {
    std::println("Usage: {} <mesh.obj> <width> <height> <output.png> <mode> [texture]", programName);
    std::println("\tMode: shaded, unlit, normals, depth, texcoords");
}

int main(int argc, char** argv) try {
    std::string programName = argv[0];
    if (argc == 2 && std::string(argv[1]) == "--help") {
        printHelp(programName);
        return EXIT_SUCCESS;
    }
    if (argc < 6 || argc > 7) {
        std::println("Try {} --help", programName);
        return EXIT_FAILURE;
    }

    Renderer renderer;

    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);
    FrameBuffer frame(width, height);
    std::string optionStr(argv[5]);
    if (optionStr == "shaded")
        renderer.setRenderMode(Renderer::RenderMode::Shaded);
    else if (optionStr == "unlit")
        renderer.setRenderMode(Renderer::RenderMode::Unlit);
    else if (optionStr == "normals")
        renderer.setRenderMode(Renderer::RenderMode::Normals);
    else if (optionStr == "depth")
        renderer.setRenderMode(Renderer::RenderMode::Depth);
    else if (optionStr == "texcoords")
        renderer.setRenderMode(Renderer::RenderMode::Texcoords);
    else {
        std::println("Unrecognised mode: {}", optionStr);
        std::println("Try {} --help", programName);
        return EXIT_FAILURE;
    }

    Material material{.baseColor = Color(1, 1, 1, 1)};
    std::optional<Texture> texture;
    if (argc == 7) {
        texture = Texture::loadFromFile(argv[6]);
        material.texture = &texture.value();
    }

    Mesh mesh = Mesh::loadFromFile(argv[1]);

    constexpr float pi = std::numbers::pi;

    const vec3 eye(0.0f, 1.0f, 3.5f);
    const vec3 target(0.0f, 0.0f, 0.0f);
    const vec3 up(0.0f, 1.0f, 0.0f);

    Camera camera(eye, target, up, 60.0f * pi / 180.0f, static_cast<float>(width) / static_cast<float>(height), 0.01,
                  100);

    Scene::Lighting lighting;
    lighting.ambientColor = Color(0.15, 0.15, 0.15, 1);
    lighting.light.color = Color(0.35, 0.35, 0.35, 1);
    lighting.light.direction = normalize(vec3(1, 1, 0));
    Scene scene(camera, lighting);
    scene.objects.emplace_back(mesh, material);
    scene.objects[0].addInstance();

    renderer.drawScene(frame, scene);

    save_image_f32_png_rgb(reinterpret_cast<float*>(frame.pixels.data()), argv[4], width, height, 4, 2.2);

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
