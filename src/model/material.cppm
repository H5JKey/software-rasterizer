export module model.material;

import math;
import model.texture;

export struct Material {
    Color baseColor = Color(1, 1, 1, 1);
    const TextureWithMipLevels* texture = nullptr;
};