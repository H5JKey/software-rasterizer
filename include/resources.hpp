#pragma once
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "mesh.hpp"
#include "texture.hpp"

class Resources {
   public:
    std::unordered_map<std::string_view, Mesh> meshes;
    std::unordered_map<std::string_view, Texture> textures;

    const Mesh& loadMesh(const std::filesystem::path& filename, std::string_view meshName);
    const Texture& loadTexture(const std::filesystem::path& filename, std::string_view textureName);
};