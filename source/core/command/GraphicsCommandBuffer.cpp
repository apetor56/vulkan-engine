#include "GraphicsCommandBuffer.hpp"
#include "QueueFamilyIDs.hpp"

namespace ve {

void GraphicsCommandBuffer::beginRenderPass( const vk::RenderPass renderPass, const vk::Framebuffer framebuffer,
                                             const vk::Extent2D renderArea ) const noexcept {
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType             = vk::StructureType::eRenderPassBeginInfo;
    renderPassBeginInfo.renderPass        = renderPass;
    renderPassBeginInfo.framebuffer       = framebuffer;
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassBeginInfo.renderArea.extent = renderArea;

    static constexpr vk::ClearValue clearColor{ { 0.1F, 0.1F, 0.1F, 1.0F } };
    renderPassBeginInfo.clearValueCount = 1U;
    renderPassBeginInfo.pClearValues    = &clearColor;

    m_commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );
}

void GraphicsCommandBuffer::bindPipeline( const vk::Pipeline pipeline ) const noexcept {
    m_commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, pipeline );
}

void GraphicsCommandBuffer::setViewport( const vk::Viewport viewport ) const noexcept {
    static constexpr std::uint32_t firstViewport{ 0U };
    m_commandBuffer.setViewport( firstViewport, viewport );
}

void GraphicsCommandBuffer::setScissor( const vk::Rect2D scissor ) const noexcept {
    static constexpr std::uint32_t firstScissor{ 0U };
    m_commandBuffer.setScissor( firstScissor, scissor );
}

void GraphicsCommandBuffer::bindVertexBuffer( const vk::Buffer vertexBuffer ) const {
    static constexpr std::uint32_t firstBinding{ 0U };
    static constexpr vk::DeviceSize offsets{ 0UL };
    m_commandBuffer.bindVertexBuffers( firstBinding, vertexBuffer, offsets );
}

void GraphicsCommandBuffer::draw( const std::uint32_t verticesCount ) const noexcept {
    static constexpr std::uint32_t instanceCount{ 1U };
    static constexpr std::uint32_t firstVertex{ 0U };
    static constexpr std::uint32_t firstInstance{ 0U };
    m_commandBuffer.draw( verticesCount, instanceCount, firstVertex, firstInstance );
}

void GraphicsCommandBuffer::endRenderPass() const noexcept {
    m_commandBuffer.endRenderPass();
}

} // namespace ve
