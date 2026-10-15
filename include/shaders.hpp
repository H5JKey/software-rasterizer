#pragma once
#include <functional>

#include "material.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "scene.hpp"

/* Vertex shaders*/

struct VertexShaderUniforms {
    const mat4& VP;
    const std::vector<mat4>& M;
    const std::vector<mat3> inverseTransposeM;
};

struct VertexShaderInput {
    vec3 position;
    vec3 normal;
    std::size_t instance_id;
};

struct VertexShaderOutput {
    vec4 position;
    vec3 normal;
};

using VertexShader = std::function<VertexShaderOutput(const VertexShaderInput&, const VertexShaderUniforms&)>;

/* Fragment shaders*/

struct FragmentShaderUniforms {
    const Scene::Lighting& lighting;
};

struct FragmentShaderInput {
    vec3 normal;
    Material material;
    vec2 uv;
    float depth;
    bool hasTexcoords;
};

struct FragmentShaderOutput {
    Color color;
};

using FragmentShader = std::function<FragmentShaderOutput(const FragmentShaderInput&, const FragmentShaderUniforms&)>;