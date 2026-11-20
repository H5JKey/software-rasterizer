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

    bool castShadow_;

   public:
    enum class Properties : unsigned { castShadow = 1u << 0 };

    static constexpr bool hasFlag(Object::Properties value, Object::Properties flag) noexcept {
        return (std::to_underlying(value) & std::to_underlying(flag)) != 0;
    }

    Mesh mesh;
    Material material;

    Object(Mesh mesh, Material material, Properties properties);
    Object(Mesh mesh, Material material);

    Object& addInstance(mat4 modelMatrix = mat4::identity());

    Object& updateTransform(std::size_t instance_id, mat4 modelMatrix);

    const std::vector<mat4>& getModel() const;

    const std::vector<mat3>& getInverseTransposeModel() const;

    std::size_t instancesCount() const;

    constexpr bool castShadow() const noexcept { return castShadow_; }
};

export constexpr Object::Properties operator|(Object::Properties a, Object::Properties b) noexcept {
    return static_cast<Object::Properties>(std::to_underlying(a) | std::to_underlying(b));
}

export constexpr Object::Properties operator&(Object::Properties a, Object::Properties b) noexcept {
    return static_cast<Object::Properties>(std::to_underlying(a) & std::to_underlying(b));
}
