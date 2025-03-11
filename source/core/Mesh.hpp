#pragma once

#include "Buffer.hpp"

#include <optional>

namespace ve {
struct MeshBuffers {
    std::optional< ve::VertexBuffer > vertexBuffer;
    std::optional< ve::IndexBuffer > indexBuffer;
    VkDeviceAddress vertexBufferAddress;
};

struct PushConstants {
    glm::mat4 worldMatrix;
    VkDeviceAddress vertexBufferAddress;

    static constexpr vk::PushConstantRange defaultRange() {
        constexpr uint32_t offset{ 0U };
        return { vk::ShaderStageFlagBits::eVertex, offset, sizeof( PushConstants ) };
    }
};

struct MeshAsset {
    MeshBuffers buffers{};
    std::string name{};
};

} // namespace ve
