#pragma once
#include <functional>

#include "material.hpp"
#include "math/matrices.hpp"
#include "math/vectors.hpp"
#include "scene.hpp"

/* Vertex shaders*/

struct VertexShaderUniforms {
    const mat4& MVP;
};

struct VertexShaderInput {
    vec3 worldPosition;
};

struct VertexShaderOutput {
    vec4 clipCoords;
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
};

struct FragmentShaderOutput {
    Color color;
};

using FragmentShader = std::function<FragmentShaderOutput(const FragmentShaderInput&, const FragmentShaderUniforms&)>;
