#include "CommandBuffer.hpp"
#include "QueueFamilyIDs.hpp"

namespace ve {

CommandBuffer::CommandBuffer( const vk::CommandBuffer& commandBufferHandler, const ve::Swapchain& swapchain,
                              const ve::Pipeline& pipeline )
    : m_commandBuffer{ commandBufferHandler }, m_swapchain{ swapchain }, m_pipeline{ pipeline } {}

void CommandBuffer::record( const uint32_t imageIndex ) const {
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;

    m_commandBuffer.begin( beginInfo );

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType             = vk::StructureType::eRenderPassBeginInfo;
    renderPassBeginInfo.renderPass        = m_swapchain.getRenderpass();
    renderPassBeginInfo.framebuffer       = m_swapchain.getFrambuffer( imageIndex );
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassBeginInfo.renderArea.extent = m_swapchain.getExtent();

    static constexpr vk::ClearValue clearColor{ { 0.1F, 0.1F, 0.1F, 1.0F } };
    renderPassBeginInfo.clearValueCount = 1U;
    renderPassBeginInfo.pClearValues    = &clearColor;

    m_commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );

    m_commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, m_pipeline.getHandler() );
    m_commandBuffer.setViewport( 0U, m_pipeline.getViewport() );
    m_commandBuffer.setScissor( 0U, m_pipeline.getScissor() );

    static constexpr std::uint32_t vertexCount{ 3U };
    static constexpr std::uint32_t instanceCount{ 1U };
    static constexpr std::uint32_t firstVertex{ 0U };
    static constexpr std::uint32_t firstInstance{ 0U };
    m_commandBuffer.draw( vertexCount, instanceCount, firstVertex, firstInstance );

    m_commandBuffer.endRenderPass();

    m_commandBuffer.end();
}

void CommandBuffer::reset() {
    vkResetCommandBuffer( m_commandBuffer, 0 );
}

vk::CommandBuffer CommandBuffer::getHandler() const noexcept {
    return m_commandBuffer;
}

} // namespace ve
