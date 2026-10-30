module;
#include <stdexcept>
#include <utility>
#include <vector>

export module scene.scene;
import scene.camera;
import model.object;
import math;

export struct DirectionalLight {
    vec3 direction;
    Color color;
};

export class Scene {
    std::vector<Object> objects;

   public:
    struct Statistics {
        std::size_t objectsCount = 0;
        std::size_t instancesCount = 0;
        std::size_t totalTrianglesCount = 0;
    } stats;
    Camera camera;
    struct Lighting {
        DirectionalLight light;
        Color ambientColor;
    };

    void addObject(Object object) {
        objects.emplace_back(std::move(object));
        stats.objectsCount += 1;
        stats.instancesCount += object.instancesCount();
        stats.totalTrianglesCount += object.instancesCount() * (object.mesh.indices.size() / 3);
    }

    const Object& getObject(std::size_t idx) const {
        if (idx >= objects.size()) throw std::out_of_range("Scene object index out of range");
        return objects[idx];
    }

    void updateObjectTransform(std::size_t object_idx, std::size_t instance_id, mat4 modelMatrix) {
        if (object_idx >= objects.size()) throw std::out_of_range("Scene object index out of range");
        objects[object_idx].updateTransform(instance_id, modelMatrix);
    }

    void addObjectInstance(std::size_t idx, mat4 modelMatrix = mat4::identity()) {
        if (idx >= objects.size()) throw std::out_of_range("Scene object index out of range");
        stats.instancesCount += 1;
        stats.totalTrianglesCount += objects[idx].mesh.indices.size() / 3;
        objects[idx].addInstance(modelMatrix);
    }

    const std::vector<Object>& getAllObjects() const { return objects; }

    Lighting lighting;
    Scene(const Camera& camera, const Lighting& lighting, std::vector<Object> objects = {})
        : objects(std::move(objects)), camera(camera), lighting(lighting) {}
};