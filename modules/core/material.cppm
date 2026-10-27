export module core.material;

import math.vectors;
import core.texture;

export struct Material {
    Color baseColor = Color(1, 1, 1, 1);
    const Texture* texture = nullptr;
};