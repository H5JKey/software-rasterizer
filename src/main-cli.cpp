
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <numbers>
#include <optional>
#include <print>
#include <string>

import scene.camera;
import renderer.frameBuffer;
import model.material;
import model.mesh;
import model.texture;
import math.transforms;
import math.vectors;
import renderer.rasterizer;
import scene.scene;
import utils.images;

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

    Rasterizer rasterizer;

    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);
    FrameBuffer frame(width, height);
    std::string optionStr(argv[5]);
    if (optionStr == "shaded")
        rasterizer.setRenderMode(Rasterizer::RenderMode::Shaded);
    else if (optionStr == "unlit")
        rasterizer.setRenderMode(Rasterizer::RenderMode::Unlit);
    else if (optionStr == "normals")
        rasterizer.setRenderMode(Rasterizer::RenderMode::Normals);
    else if (optionStr == "depth")
        rasterizer.setRenderMode(Rasterizer::RenderMode::Depth);
    else if (optionStr == "texcoords")
        rasterizer.setRenderMode(Rasterizer::RenderMode::Texcoords);
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

    rasterizer.drawScene(frame, scene);

    save_image_f32_png_rgb(reinterpret_cast<float*>(frame.pixels.data()), argv[4], width, height, 4, 2.2);

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
