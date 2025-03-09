#pragma once

#include "Pipeline.hpp"

namespace ve {

struct Material {
    enum class Type { eMainColor, eTransparent, eOther };

    const ve::Pipeline& pipeline;
    vk::DescriptorSet set;
};

} // namespace ve
