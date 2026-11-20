module;

#include <filesystem>
#include <cmath>

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
        float minDepth = std::numeric_limits<float>::infinity();
        float maxDepth = -std::numeric_limits<float>::infinity();
        auto& depth = buffer.depth();
        for (float d : depth) {
            if (std::isfinite(d)) {
                minDepth = std::min(minDepth, d);
                maxDepth = std::max(maxDepth, d);
            }
        }
        std::vector<float> normalized(3 * buffer.depth().size());
        for (int i=0; i<buffer.depth().size(); i++) {
            float normalizedDepth = (depth[i] - minDepth) / (maxDepth - minDepth);
            normalized[3 * i] = normalizedDepth;
            normalized[3 * i + 1] = normalizedDepth;
            normalized[3 * i + 2] = normalizedDepth;
        }
        save_image_f32_png_rgb(normalized.data(), filename, buffer.width, buffer.height, 3, 1.0f);
    }
};