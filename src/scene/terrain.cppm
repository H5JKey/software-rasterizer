module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <print>
#include <vector>

export module scene.terrain;

import math;
import utils.perlin;
import model.object;
import model.mesh;
import model.material;
import model.material.shaders;
import model.texture;

export class Terrain {
   public:
    int resolution = 256;       // число клеток сетки
    float step = 0.5f;          // размер шага клетки
    float heightScale = 16.0f;  // амплитуда высот
    float invUvPeriod = 0.5f;   // 1.0 / период повторения uv
   private:
    Perlin2D heightMap;
    Object terrainObject;

    Mesh buildGeometry() const {
        Mesh geometry;
        std::size_t n = std::max(resolution, 1);
        std::size_t row = resolution + 1;
        float halfResolution = static_cast<float>(resolution) * 0.5f;

        std::vector<float> heights(row * row);
        for (std::size_t j = 0; j <= n; j++) {
            for (std::size_t i = 0; i <= n; ++i) {
                float x = (static_cast<float>(i) - halfResolution) * step;
                float z = (static_cast<float>(j) - halfResolution) * step;
                heights[j * row + i] = heightAt(x, z);
            }
        }

        geometry.vertices.reserve(row * row);
        geometry.normals.resize(row * row);
        geometry.indices.reserve(row * row * 6);

        for (std::size_t j = 0; j <= n; ++j) {
            float z = (static_cast<float>(j) - halfResolution) * step;
            for (std::size_t i = 0; i <= n; ++i) {
                float x = (static_cast<float>(i) - halfResolution) * step;

                geometry.vertices.emplace_back(x, 0, z);
            }
        }

        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                const int a = j * row + i;
                const int b = a + 1;
                const int c = a + row;
                const int d = c + 1;

                geometry.indices.push_back(a);
                geometry.indices.push_back(c);
                geometry.indices.push_back(b);
                geometry.indices.push_back(b);
                geometry.indices.push_back(c);
                geometry.indices.push_back(d);
            }
        }
        return geometry;
    }

   public:
    Terrain(std::uint32_t seed, const std::filesystem::path& grass, const std::filesystem::path& stone)
        : heightMap(seed, 4),
          terrainObject(buildGeometry(), Material{
                                             .baseColor = Color(1, 1, 1, 1),
                                             .textures = {&TextureWithMipLevels::loadFromFile(grass),
                                                          &TextureWithMipLevels::loadFromFile(stone)},
                                             .shader = shaders::TerrainTexturesMaterialShader,
                                         }) {
        terrainObject.addInstance();
    }

    float heightAt(float x, float z) const {
        const vec2 p(x * 0.05f, z * 0.05f);
        return heightMap.sample(p) * heightScale;
    }

    const Object& asObject() const noexcept { return terrainObject; }

    void followCamera(vec3 cameraPosition) {
        const float snapX = std::round(cameraPosition.x / step) * step;
        const float snapZ = std::round(cameraPosition.z / step) * step;
        terrainObject.updateTransform(0, translation(snapX, 0.0f, snapZ));
    }
};
