module;
#include <stdexcept>
#include <utility>
#include <vector>

export module core.object;
import core.material;
import math.matrices;
import core.mesh;

export class Object {
    std::vector<mat4> modelMatrices;
    std::vector<mat3> inverseTransposeModelMatrices;

   public:
    Mesh mesh;
    Material material;

    Object(Mesh mesh, Material material) : mesh(std::move(mesh)), material(std::move(material)) {}

    void addInstance(mat4 modelMatrix = mat4::identity()) {
        inverseTransposeModelMatrices.push_back(modelMatrix.upperLeft<3>().inverse().transpose());
        modelMatrices.push_back(std::move(modelMatrix));
    }

    void updateTransform(std::size_t instance_id, mat4 modelMatrix) {
        if (instance_id >= instancesCount()) {
            throw std::invalid_argument("Incorrect instance_id");
        }
        inverseTransposeModelMatrices[instance_id] = modelMatrix.upperLeft<3>().inverse().transpose();
        modelMatrices[instance_id] = std::move(modelMatrix);
    }

    const std::vector<mat4>& getModel() const { return modelMatrices; }

    const std::vector<mat3>& getInverseTransposeModel() const { return inverseTransposeModelMatrices; }

    std::size_t instancesCount() const { return modelMatrices.size(); }
};
