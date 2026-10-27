module;
#include <utility>
#include <vector>

export module scene.scene;
import scene.camera;
import model.object;
import math.vectors;

export struct DirectionalLight {
    vec3 direction;
    Color color;
};

export struct Scene {
    std::vector<Object> objects;
    Camera camera;
    struct Lighting {
        DirectionalLight light;
        Color ambientColor;
    };

    Lighting lighting;

    Scene(const Camera& camera, const Lighting& lighting, std::vector<Object> objects = {})
        : objects(std::move(objects)), camera(camera), lighting(lighting) {}
};