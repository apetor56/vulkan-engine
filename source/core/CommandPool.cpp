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

std::vector< ve::CommandBuffer > CommandPool::createCommandBuffers( const std::uint32_t count ) {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = count;

    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto commandBufferHandlers{ logicalDeviceHandler.allocateCommandBuffers( allocInfo ) };

    std::vector< ve::CommandBuffer > m_commandBuffers;
    for ( std::uint32_t index{ 0 }; index < count; index++ )
        m_commandBuffers.emplace_back( commandBufferHandlers.at( index ), m_swapchain, m_pipeline );

    return m_commandBuffers;
}

} // namespace ve
