#include "CommandBuffer.hpp"
#include "QueueFamilyIndices.hpp"

#include <stdexcept>

namespace ve {

CommandBuffer::CommandBuffer( const ve::PhysicalDevice& physicalDevice, const ve::LogicalDevice& logicalDevice,
                              const ve::Swapchain& swapchain, const ve::Pipeline& pipeline, const ve::Window& window )
    : m_physicalDevice{ physicalDevice },
      m_logicalDevice{ logicalDevice },
      m_swapchain{ swapchain },
      m_pipeline{ pipeline },
      m_window{ window } {
    createCommandPool();
    createCommandBuffer();
}

CommandBuffer::~CommandBuffer() {
    vkDestroyCommandPool( m_logicalDevice.getHandler(), m_commandPool, nullptr );
}

void CommandBuffer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices{
        QueueFamilyIndices::findQueueFamilies( m_physicalDevice.getHandler(), m_window.getSurface() ) };

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if ( vkCreateCommandPool( m_logicalDevice.getHandler(), &poolInfo, nullptr, &m_commandPool ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to create command pool!" );
    }
}

void CommandBuffer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if ( vkAllocateCommandBuffers( m_logicalDevice.getHandler(), &allocInfo, &m_commandBuffer ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to allocate command buffers!" );
    }
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

    const auto& viewport{ m_pipeline.getViewport() };
    vkCmdSetViewport( m_commandBuffer, 0, 1, &viewport );
    const auto& scissor{ m_pipeline.getScissor() };
    vkCmdSetScissor( m_commandBuffer, 0, 1, &scissor );

    vkCmdDraw( m_commandBuffer, 3, 1, 0, 0 );
    vkCmdEndRenderPass( m_commandBuffer );

    if ( vkEndCommandBuffer( m_commandBuffer ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to record command buffer" );
    }
}

void CommandBuffer::reset() {
    vkResetCommandBuffer( m_commandBuffer, 0 );
}

VkCommandBuffer CommandBuffer::getHandler() const noexcept {
    return m_commandBuffer;
}

} // namespace ve