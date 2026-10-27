module;
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

module core.object;

import core.material;
import math.matrices;
import core.mesh;
import math.vectors;

Object::Object(Mesh mesh, Material material) : mesh(std::move(mesh)), material(std::move(material)) {}

void Object::addInstance(mat4 modelMatrix) {
    inverseTransposeModelMatrices.push_back(modelMatrix.upperLeft<3>().inverse().transpose());
    modelMatrices.push_back(std::move(modelMatrix));
}

void Object::updateTransform(std::size_t instance_id, mat4 modelMatrix) {
    if (instance_id >= instancesCount()) {
        throw std::invalid_argument("Incorrect instance_id");
    }
    inverseTransposeModelMatrices[instance_id] = modelMatrix.upperLeft<3>().inverse().transpose();
    modelMatrices[instance_id] = std::move(modelMatrix);
}

const std::vector<mat4>& Object::getModel() const { return modelMatrices; }

const std::vector<mat3>& Object::getInverseTransposeModel() const { return inverseTransposeModelMatrices; }

std::size_t Object::instancesCount() const { return modelMatrices.size(); }
