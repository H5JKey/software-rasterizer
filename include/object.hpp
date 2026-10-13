#pragma once
#include "material.hpp"
#include "math/matrices.hpp"
#include "mesh.hpp"

class Object {
    mat4 modelMatrix;
    mat4 transform = mat4::identity();

   public:
    const Mesh& mesh;
    const Material& material;

    Object(const Mesh& mesh, const Material& material)
        : modelMatrix(mesh.normalizationMatrix), mesh(mesh), material(material) {}
    void updateTransform(mat4 transform) {
        this->transform = std::move(transform);
        modelMatrix = transform * mesh.normalizationMatrix;
    }
    const mat4& getModelMatrix() const { return modelMatrix; }
};