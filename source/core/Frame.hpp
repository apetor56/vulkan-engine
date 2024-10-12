#pragma once

#include "SyncObjects.hpp"
#include "Buffer.hpp"
#include "command/GraphicsCommandBuffer.hpp"

#include <optional>

namespace ve {

struct Frame {
    Frame( const ve::LogicalDevice& _logicalDevice, const ve::MemoryAllocator& allocator,
           const ve::GraphicsCommandBuffer _commandBuffer, const vk::DescriptorSet _descriptorSet )
        : uniformBuffer{ allocator, sizeof( UniformBufferData ) },
          swapchainSemaphore{ _logicalDevice },
          renderSemaphore{ _logicalDevice },
          renderFence{ _logicalDevice },
          graphicsCommandBuffer{ _commandBuffer },
          descriptorSet{ _descriptorSet } {}

    Frame( const Frame& other ) = delete;
    Frame( Frame&& other )      = delete;

    Frame& operator=( const Frame& other ) = delete;
    Frame& operator=( Frame&& other )      = delete;

    ve::UniformBuffer uniformBuffer;
    ve::Semaphore swapchainSemaphore;
    ve::Semaphore renderSemaphore;
    ve::Fence renderFence;
    ve::GraphicsCommandBuffer graphicsCommandBuffer;
    vk::DescriptorSet descriptorSet;
};

} // namespace ve