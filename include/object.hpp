#pragma once
#include "math/matrices.hpp"
#include "mesh.hpp"

class Object {
   public:
    const Mesh& mesh;
    mat4 transform;

    Object(const Mesh& mesh, const mat4& transform) : mesh(mesh), transform(transform) {}
    void setTransform(const mat4& transform) { this->transform = transform; }
};