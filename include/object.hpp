#pragma once
#include "math/matrices.hpp"
#include "mesh.hpp"

class Object {
    mat4 modelMatrix;
    mat4 transform = mat4::identity();

   public:
    const Mesh& mesh;

    Object(const Mesh& mesh) : modelMatrix(mesh.normalizationMatrix), mesh(mesh) {}
    void updateTransform(const mat4& transform) {
        this->transform = transform;
        modelMatrix = transform * mesh.normalizationMatrix;
    }
    const mat4& getModelMatrix() const { return modelMatrix; }
};