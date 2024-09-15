#pragma once

#include <vulkan/vulkan.hpp>

namespace ve {

class BaseCommandBuffer {
public:
    BaseCommandBuffer( const vk::CommandBuffer commandBufferHandler ) noexcept;
    ~BaseCommandBuffer() = default;

    BaseCommandBuffer( const BaseCommandBuffer& other ) = delete;
    BaseCommandBuffer( BaseCommandBuffer&& other )      = default;

    BaseCommandBuffer& operator=( const BaseCommandBuffer& other ) = delete;
    BaseCommandBuffer& operator=( BaseCommandBuffer&& other )      = delete;

    void begin() const;
    void end() const;
    void reset();

    vk::CommandBuffer getHandler() const noexcept;

protected:
    vk::CommandBuffer m_commandBuffer;
};
} // namespace ve
