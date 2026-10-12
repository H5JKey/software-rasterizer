#pragma once
#include "math/matrices.hpp"
#include "mesh.hpp"
#include "texture.hpp"

struct Material {
    const Texture* texture = nullptr;
    Color baseColor = Color(1, 1, 1, 1);
};

class Object {
    mat4 modelMatrix;
    mat4 transform = mat4::identity();

   public:
    const Mesh& mesh;
    Material material;

    Object(const Mesh& mesh, Material material)
        : modelMatrix(mesh.normalizationMatrix), mesh(mesh), material(std::move(material)) {}
    void updateTransform(mat4 transform) {
        this->transform = std::move(transform);
        modelMatrix = transform * mesh.normalizationMatrix;
    }
    const mat4& getModelMatrix() const { return modelMatrix; }
};