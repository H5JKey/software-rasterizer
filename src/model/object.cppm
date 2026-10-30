module;
#include <cstddef>
#include <vector>

export module model.object;
import model.material;
import model.mesh;
import math;

export class Object {
    std::vector<mat4> modelMatrices;
    std::vector<mat3> inverseTransposeModelMatrices;

   public:
    Mesh mesh;
    Material material;

    Object(Mesh mesh, Material material);

    void addInstance(mat4 modelMatrix = mat4::identity());

    void updateTransform(std::size_t instance_id, mat4 modelMatrix);

    const std::vector<mat4>& getModel() const;

    const std::vector<mat3>& getInverseTransposeModel() const;

    std::size_t instancesCount() const;
};
