module;
#include <filesystem>

export module scene.skybox;
import model.texture;
import math;

export class Skybox {
    Texture left, right, front, back, top, bottom;

   public:
    Skybox(const std::filesystem::path& leftPath, const std::filesystem::path& rightPath,
           const std::filesystem::path& frontPath, const std::filesystem::path& backPath,
           const std::filesystem::path& topPath, const std::filesystem::path& bottomPath)
        : left(Texture::loadFromFile(leftPath)),
          right(Texture::loadFromFile(rightPath)),
          front(Texture::loadFromFile(frontPath)),
          back(Texture::loadFromFile(backPath)),
          top(Texture::loadFromFile(topPath)),
          bottom(Texture::loadFromFile(bottomPath)) {}

    Color sample(const vec3& direction) const {
        const float ax = std::abs(direction.x);
        const float ay = std::abs(direction.y);
        const float az = std::abs(direction.z);

        vec2 uv;
        const Texture* tex;

        if (ax >= ay && ax >= az) {
            float m = ax;
            if (direction.x > 0) {
                /*right*/
                uv = vec2((-direction.z / m + 1.0f) * 0.5f, (direction.y / m + 1.0f) * 0.5f);
                tex = &right;
            } else {
                /*left*/
                uv = vec2((direction.z / m + 1.0f) * 0.5f, (direction.y / m + 1.0f) * 0.5f);
                tex = &left;
            }
        } else if (ay >= ax && ay >= az) {
            float m = ay;
            if (direction.y > 0) {
                /*top*/
                uv = vec2((-direction.x / m + 1.0f) * 0.5f, (direction.z / m + 1.0f) * 0.5f);
                tex = &top;
            } else {
                /*bottom*/
                uv = vec2((direction.x / m + 1.0f) * 0.5f, (-direction.z / m + 1.0f) * 0.5f);
                tex = &bottom;
            }
        } else {
            float m = az;
            if (direction.z > 0) {
                /*back*/
                uv = vec2((direction.x / m + 1.0f) * 0.5f, (direction.y / m + 1.0f) * 0.5f);
                tex = &back;
            } else {
                /*front*/
                uv = vec2((-direction.x / m + 1.0f) * 0.5f, (direction.y / m + 1.0f) * 0.5f);
                tex = &front;
            }
        }

        /*сдвигаем uv на пол текстеля, чтобы избежать видимых границ*/
        const float halfX = 0.5f / static_cast<float>(tex->width);
        const float halfY = 0.5f / static_cast<float>(tex->height);

        uv.x = std::clamp(uv.x, halfX, 1.0f - halfX);
        uv.y = std::clamp(uv.y, halfY, 1.0f - halfY);

        return tex->sample(uv);
    }
};