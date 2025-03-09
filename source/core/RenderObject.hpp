#pragma once

#include "Buffer.hpp"
#include "Material.hpp"

namespace ve {

class RenderContext;

struct RenderObject {
    const uint32_t indexCount{};
    const uint32_t firstIndex{};
    const ve::IndexBuffer indexBuffer;
    const ve::Material& material;
    glm::mat4 transform;
};

struct Renderable {
    virtual void render( const glm::mat4& topMatrix, RenderContext& renderContext ) = 0;
};

} // namespace ve
