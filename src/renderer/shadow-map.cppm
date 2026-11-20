module;

#include <filesystem>

export module renderer.shadowmap;
import renderer.framebuffer;
import utils.images;
import math;

export class ShadowMap {
   public:
    mat4 lightVP;
    float bias = 1e-3f;
    int resolution;
    bool enabled = false;
    Framebuffer buffer;

    ShadowMap() : resolution(0), buffer(0, 0, Framebuffer::Attachments::Depth) {}

    ShadowMap(int resolution)
        : resolution(resolution), buffer(resolution, resolution, Framebuffer::Attachments::Depth) {}

    void saveToPng(const std::filesystem::path& filename) const {
        save_image_f32_png_rgb(buffer.depth().data(), filename, buffer.width, buffer.height, 1, 1.0f);
    }
};