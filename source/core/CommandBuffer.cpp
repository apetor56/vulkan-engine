#include "CommandBuffer.hpp"
#include "QueueFamilyIndices.hpp"

#include <stdexcept>

namespace ve {

CommandBuffer::CommandBuffer( const ve::PhysicalDevice& physicalDevice, const ve::LogicalDevice& logicalDevice,
                              const ve::Swapchain& swapchain, const ve::Pipeline& pipeline )
    : m_physicalDevice{ physicalDevice },
      m_logicalDevice{ logicalDevice },
      m_swapchain{ swapchain },
      m_pipeline{ pipeline } {
    createCommandPool();
    createCommandBuffer();
}

CommandBuffer::~CommandBuffer() {
    vkDestroyCommandPool( m_logicalDevice.getHandler(), m_commandPool, nullptr );
}

void CommandBuffer::createCommandPool() {
    QueueFamilyIndices queueFamilies{ m_physicalDevice.getQueueFamilies() };

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilies.graphicsFamilyID.value();

    if ( vkCreateCommandPool( m_logicalDevice.getHandler(), &poolInfo, nullptr, &m_commandPool ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to create command pool!" );
    }
}

void CommandBuffer::createCommandBuffer() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    m_commandBuffer = logicalDeviceHandler.allocateCommandBuffers( allocInfo ).at( 0 );
}

void CommandBuffer::record( const uint32_t imageIndex ) const {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = 0u;
    beginInfo.pInheritanceInfo = nullptr;

    if ( vkBeginCommandBuffer( m_commandBuffer, &beginInfo ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to begin recording command buffer!" );
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = m_swapchain.getRenderpass();
    renderPassInfo.framebuffer       = m_swapchain.getFrambuffer( imageIndex );
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchain.getExtent();

    constexpr VkClearValue clearColor{ .color{ 0.1f, 0.1f, 0.1f, 1.0f } };

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues    = &clearColor;

    vkCmdBeginRenderPass( m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdBindPipeline( m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getHandler() );

    const auto viewport{ m_pipeline.getViewport() };
    m_commandBuffer.setViewport( 0U, viewport );
    const auto scissor{ m_pipeline.getScissor() };
    m_commandBuffer.setScissor( 0U, scissor );

    vkCmdDraw( m_commandBuffer, 3, 1, 0, 0 );
    vkCmdEndRenderPass( m_commandBuffer );

    if ( vkEndCommandBuffer( m_commandBuffer ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to record command buffer" );
    }
}

void CommandBuffer::reset() {
    vkResetCommandBuffer( m_commandBuffer, 0 );
}

vk::CommandBuffer CommandBuffer::getHandler() const noexcept {
    return m_commandBuffer;
}

} // namespace ve
