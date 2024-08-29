#include "CommandPool.hpp"

namespace ve {

CommandPool::CommandPool( const ve::LogicalDevice& logicalDevice, const ve::Swapchain& swapchain,
                          const ve::Pipeline& pipeline )
    : m_logicalDevice{ logicalDevice }, m_swapchain{ swapchain }, m_pipeline{ pipeline } {
    createCommandPool();
}

CommandPool::~CommandPool() {
    m_logicalDevice.getHandler().destroyCommandPool( m_commandPool );
}

void CommandPool::createCommandPool() {
    ve::QueueFamilyIDs queueFamilies{ m_logicalDevice.getQueueFamilyIDs() };

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = vk::StructureType::eCommandPoolCreateInfo;
    poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueFamilies.graphicsFamilyID.value();

    m_commandPool = m_logicalDevice.getHandler().createCommandPool( poolInfo );
}

ve::CommandBuffer CommandPool::createCommandBuffer() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1U;

    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    vk::CommandBuffer commandBufferHandler{ logicalDeviceHandler.allocateCommandBuffers( allocInfo ).at( 0 ) };

    return { commandBufferHandler, m_swapchain, m_pipeline };
}

} // namespace ve
