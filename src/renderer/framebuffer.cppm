module;
#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>
#include <cassert>
#include <utility>

export module renderer.framebuffer;

import math;
import renderer.image;


export enum class Attachments : unsigned{ 
    Color = 1u<<0, 
    Depth = 1u<<1 
};

export constexpr Attachments operator|(Attachments a, Attachments b) noexcept {
    return static_cast<Attachments>(std::to_underlying(a) | std::to_underlying(b));
}

export constexpr Attachments operator&(Attachments a, Attachments b) noexcept {
    return static_cast<Attachments>(std::to_underlying(a) & std::to_underlying(b));
}

export constexpr bool hasFlag(Attachments value, Attachments flag) noexcept {
    return (std::to_underlying(value) & std::to_underlying(flag)) != 0;
}

export class Framebuffer {
    bool hasDepth_, hasColor_;
   public:
    int width, height;
    Image image_;
    std::vector<float> depth_;

    Framebuffer(int width, int height, Attachments attachments)
        : hasDepth_(hasFlag(attachments, Attachments::Depth)),
          hasColor_(hasFlag(attachments, Attachments::Color)),
          width(width),
          height(height),
          image_(hasColor_ ? width : 0, hasColor_ ? height : 0),
          depth_(hasDepth_ ? width * height : 0, std::numeric_limits<float>::infinity()) {}

    void resize(int width, int height) {
        this->width = width;
        this->height = height;
        if (hasColor_)
            image_.resize(width, height);
        if (hasDepth_)
            depth_.resize(width * height);
    }

    bool hasColor() const noexcept {return hasColor_;}
    bool hasDepth() const noexcept {return hasDepth_;}

    Image& image() {
        assert(hasColor_, "image() called without color attachment");
        return image_;
    
    }
    std::vector<float>& depth() {
        assert(hasColor_, "depth() called without depth attachment");
        return depth_;
    }

    void clear() {
        if (hasColor_)
            image_.clear();
        if (hasDepth_)
            std::fill(depth_.begin(), depth_.end(), std::numeric_limits<float>::infinity());
    }
};