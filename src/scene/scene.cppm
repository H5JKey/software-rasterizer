module;
#include <stdexcept>
#include <utility>
#include <vector>

export module scene.scene;
import scene.camera;
import model.object;
import scene.skybox;
import math;
import scene.terrain;

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

    struct Lighting {
        DirectionalLight light;
        Color ambientColor;
    };
    Skybox skybox;
    Camera camera;
    Terrain terrain;

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
    Scene(Camera camera, Lighting lighting, Skybox skybox, Terrain terrain, std::vector<Object> objects = {})
        : objects(std::move(objects)),
          skybox(std::move(skybox)),
          camera(std::move(camera)),
          terrain(std::move(terrain)),
          lighting(std::move(lighting)) {
        /* Учитываем ладшафт*/
        stats.objectsCount += 1;
        stats.instancesCount += 1;
        stats.totalTrianglesCount += terrain.asObject().mesh.indices.size() / 3;
    }

    void update(double elapsedTime) {
        camera.update(elapsedTime);

        terrain.followCamera(camera.getEye());
    }
};