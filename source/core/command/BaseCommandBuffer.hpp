#pragma once

#include <vulkan/vulkan.hpp>

namespace ve {

class BaseCommandBuffer {
public:
    BaseCommandBuffer( const vk::CommandBuffer commandBufferHandler ) noexcept;
    ~BaseCommandBuffer() = default;

    BaseCommandBuffer( const BaseCommandBuffer& other ) = default;
    BaseCommandBuffer( BaseCommandBuffer&& other )      = default;

    BaseCommandBuffer& operator=( const BaseCommandBuffer& other ) = delete;
    BaseCommandBuffer& operator=( BaseCommandBuffer&& other )      = delete;

    void begin( const vk::CommandBufferUsageFlags flags = {} ) const;
    void end() const;
    void reset() const;

    vk::CommandBuffer get() const noexcept;

protected:
    vk::CommandBuffer m_commandBuffer;
};
} // namespace ve
