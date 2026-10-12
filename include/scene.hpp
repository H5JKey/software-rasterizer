#pragma once
#include <vector>

#include "camera.hpp"
#include "object.hpp"

struct DirectionalLight {
    vec3 direction;
    Color color;
};

struct Scene {
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