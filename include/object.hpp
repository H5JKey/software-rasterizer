#pragma once
#include <print>
#include <stdexcept>

#include "material.hpp"
#include "math/matrices.hpp"
#include "mesh.hpp"
class Object {
   public:
    Mesh mesh;
    Material material;
    std::vector<mat4> modelMatrices;
    std::vector<mat3> inverseTransposeModelMatrices;

    Object(Mesh mesh, Material material) : mesh(std::move(mesh)), material(std::move(material)) {}

    void addInstance(mat4 modelMatrix = mat4::identity()) {
        inverseTransposeModelMatrices.push_back(modelMatrix.upperLeft<3>().inverse().transpose());
        modelMatrices.push_back(modelMatrix);
    }

    void updateTransform(std::size_t instance_id, mat4 modelMatrix) {
        if (instance_id >= modelMatrices.size() || instance_id < 0)
            throw std::invalid_argument("Incorrect instance_id");
        inverseTransposeModelMatrices[instance_id] = modelMatrix.upperLeft<3>().inverse().transpose();
        modelMatrices[instance_id] = std::move(modelMatrix);
    }
};