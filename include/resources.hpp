#pragma once
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "material.hpp"
#include "mesh.hpp"

class Resources {
   public:
    std::unordered_map<std::string_view, Mesh> meshes;
    std::unordered_map<std::string_view, Texture> textures;
    std::unordered_map<std::string_view, Material> materials;

    const Mesh& loadMesh(const std::filesystem::path& filename, std::string_view meshName);
    const Texture& loadTexture(const std::filesystem::path& filename, std::string_view textureName);
};