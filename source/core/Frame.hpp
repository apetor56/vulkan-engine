#pragma once

#include "SyncObjects.hpp"
#include "Buffer.hpp"
#include "command/GraphicsCommandBuffer.hpp"
#include "descriptor/DescriptorAllocator.hpp"

#include <optional>

namespace ve {

struct Frame {
    Frame( const ve::LogicalDevice& _logicalDevice, const ve::MemoryAllocator& _memoryAllocator,
           const ve::GraphicsCommandBuffer _commandBuffer, const ve::DescriptorSetLayout& _layout );

    Frame( const Frame& other ) = delete;
    Frame( Frame&& other )      = delete;

    Frame& operator=( const Frame& other ) = delete;
    Frame& operator=( Frame&& other )      = delete;

    ve::UniformBuffer uniformBuffer;
    ve::Semaphore swapchainSemaphore;
    ve::Semaphore renderSemaphore;
    ve::Fence renderFence;
    ve::GraphicsCommandBuffer graphicsCommandBuffer;
    ve::DescriptorAllocator descriptorAllocator;
    vk::DescriptorSet descriptorSet;
};

} // namespace ve