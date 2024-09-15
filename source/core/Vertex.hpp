#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec3.hpp>

#include <array>

namespace ve {

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;

    static constexpr vk::VertexInputBindingDescription getBindingDescription() noexcept {
        constexpr auto bindingPoint{ 0U };
        constexpr auto stride{ sizeof( Vertex ) };
        constexpr auto inputRate{ vk::VertexInputRate::eVertex };

        return { bindingPoint, stride, inputRate };
    }

    static constexpr std::array< vk::VertexInputAttributeDescription, 2U > getAttributeDescripstions() noexcept {
        constexpr auto bindingPoint{ 0U };
        constexpr auto positionLocation{ 0U };
        constexpr auto colorLocation{ 1U };

        return { vk::VertexInputAttributeDescription{ positionLocation, bindingPoint, vk::Format::eR32G32B32Sfloat,
                                                      offsetof( Vertex, position ) },
                 vk::VertexInputAttributeDescription{ colorLocation, bindingPoint, vk::Format::eR32G32B32Sfloat,
                                                      offsetof( Vertex, color ) } };
    }
};

static const std::vector< Vertex > temporaryVertices{ Vertex{ { 0.0F, -0.5F, 0.0F }, { 1.0F, 0.0F, 0.0F } },
                                                      { { 0.5F, 0.5F, 0.0F }, { 0.0F, 1.0F, 0.0F } },
                                                      { { -0.5F, 0.5F, 0.0F }, { 0.0F, 0.0F, 1.0F } } };

} // namespace ve
