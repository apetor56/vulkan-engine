#include "Frame.hpp"

namespace ve {

static std::array< ve::DescriptorAllocator::PoolSizeRatio, 4U > g_poolSizeRatios{
    ve::DescriptorAllocator::PoolSizeRatio{ vk::DescriptorType::eStorageImage, 0.3F },
    { vk::DescriptorType::eStorageBuffer, 0.3F },
    { vk::DescriptorType::eUniformBuffer, 0.3F },
    { vk::DescriptorType::eCombinedImageSampler, 0.4F } };

static constexpr uint32_t g_maxSets{ 1000U };

FrameData::FrameData( const uint32_t uniformBufferSize, const ve::LogicalDevice& _logicalDevice,
                      const ve::MemoryAllocator& _memoryAllocator, const ve::GraphicsCommandBuffer _commandBuffer,
                      const ve::DescriptorSetLayout& _layout )
    : uniformBuffer{ _memoryAllocator, uniformBufferSize },
      swapchainSemaphore{ _logicalDevice },
      renderSemaphore{ _logicalDevice },
      renderFence{ _logicalDevice },
      graphicsCommandBuffer{ _commandBuffer },
      descriptorAllocator{ _logicalDevice, g_maxSets, g_poolSizeRatios },
      descriptorSet{ descriptorAllocator.allocate( _layout ) } {}

} // namespace ve
