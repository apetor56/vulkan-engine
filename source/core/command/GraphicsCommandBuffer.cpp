#include "GraphicsCommandBuffer.hpp"
#include "QueueFamilyIDs.hpp"

namespace {
constexpr std::uint32_t g_firstVertex{ 0U };
constexpr std::uint32_t g_instanceCount{ 1U };
constexpr std::uint32_t g_firstInstance{ 0U };
constexpr std::uint32_t g_firstIndex{ 0U };
constexpr vk::DeviceSize g_offset{ 0UL };
constexpr std::uint32_t g_firstBinding{ 0U };
constexpr std::uint32_t g_firstScissor{ 0U };
constexpr std::uint32_t g_firstViewport{ 0U };
constexpr vk::ClearValue g_clearColor{ { 0.1F, 0.1F, 0.1F, 1.0F } };
} // namespace

namespace ve {

void GraphicsCommandBuffer::beginRenderPass( const vk::RenderPass renderPass, const vk::Framebuffer framebuffer,
                                             const vk::Extent2D renderArea ) const noexcept {
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType             = vk::StructureType::eRenderPassBeginInfo;
    renderPassBeginInfo.renderPass        = renderPass;
    renderPassBeginInfo.framebuffer       = framebuffer;
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassBeginInfo.renderArea.extent = renderArea;

    renderPassBeginInfo.clearValueCount = 1U;
    renderPassBeginInfo.pClearValues    = &g_clearColor;

    m_commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );
}

void GraphicsCommandBuffer::bindPipeline( const vk::Pipeline pipeline ) const noexcept {
    m_commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, pipeline );
}

void GraphicsCommandBuffer::setViewport( const vk::Viewport viewport ) const noexcept {
    m_commandBuffer.setViewport( g_firstViewport, viewport );
}

void GraphicsCommandBuffer::setScissor( const vk::Rect2D scissor ) const noexcept {
    m_commandBuffer.setScissor( g_firstScissor, scissor );
}

void GraphicsCommandBuffer::bindVertexBuffer( const vk::Buffer vertexBuffer ) const {
    m_commandBuffer.bindVertexBuffers( g_firstBinding, vertexBuffer, g_offset );
}

void GraphicsCommandBuffer::bindIndexBuffer( const vk::Buffer indexBuffer ) const {
    m_commandBuffer.bindIndexBuffer( indexBuffer, g_offset, vk::IndexType::eUint32 );
}

void GraphicsCommandBuffer::bindDescriptorSet( const vk::PipelineLayout pipelineLayout,
                                               const vk::DescriptorSet descriptorSet,
                                               const std::uint32_t firstSet ) const noexcept {
    m_commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayout, firstSet, descriptorSet,
                                        nullptr );
}

void GraphicsCommandBuffer::draw( const std::uint32_t verticesCount ) const noexcept {
    m_commandBuffer.draw( verticesCount, g_instanceCount, g_firstVertex, g_firstInstance );
}

void GraphicsCommandBuffer::drawIndices( const std::uint32_t indicesCount ) const noexcept {
    m_commandBuffer.drawIndexed( indicesCount, g_instanceCount, g_firstIndex, g_offset, g_firstInstance );
}

void GraphicsCommandBuffer::endRenderPass() const noexcept {
    m_commandBuffer.endRenderPass();
}

} // namespace ve
