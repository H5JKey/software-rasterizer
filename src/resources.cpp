#include "resources.hpp"

#include <vector>

#include "image-utils.hpp"
#include "mesh-utils.hpp"

const Mesh& Resources::loadMesh(const std::filesystem::path& filename, std::string_view meshName) {
    meshes[meshName] = load_obj(filename);
    return meshes[meshName];
}

const Texture& Resources::loadTexture(const std::filesystem::path& filename, std::string_view textureName) {
    int width, height;
    std::vector<float> rgb_f32 = load_image_f32_rgb(filename, width, height, 2.2f);
    std::vector<Color> pixels(width * height);
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            std::size_t idx = row * height + col;
            pixels[idx] = Color(rgb_f32[3 * idx + 0], rgb_f32[3 * idx + 1], rgb_f32[3 * idx + 2], 1.0f);
        }
    }

    textures[textureName] = Texture(width, height, std::move(pixels));
    return textures[textureName];
}
