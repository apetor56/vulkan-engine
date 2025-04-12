#pragma once

#include "SyncObjects.hpp"
#include "Buffer.hpp"
#include "command/GraphicsCommandBuffer.hpp"
#include "descriptor/DescriptorAllocator.hpp"

namespace ve {

struct FrameData : public utils::NonCopyable,
                   public utils::NonMovable {
    FrameData( const uint32_t uniformBufferSize, const ve::LogicalDevice& _logicalDevice,
               const ve::MemoryAllocator& _memoryAllocator, const ve::GraphicsCommandBuffer _commandBuffer,
               const ve::DescriptorSetLayout& _layout );

    ve::UniformBuffer uniformBuffer;
    ve::Semaphore swapchainSemaphore;
    ve::Semaphore renderSemaphore;
    ve::Fence renderFence;
    ve::GraphicsCommandBuffer graphicsCommandBuffer;
    ve::DescriptorAllocator descriptorAllocator;
    vk::DescriptorSet descriptorSet;
};

} // namespace ve
