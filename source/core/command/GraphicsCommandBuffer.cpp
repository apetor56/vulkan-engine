#include "GraphicsCommandBuffer.hpp"
#include "QueueFamilyIDs.hpp"
#include "LogicalDevice.hpp"
#include "Loader.hpp"

namespace {
constexpr uint32_t g_firstVertex{ 0U };
constexpr uint32_t g_instanceCount{ 1U };
constexpr uint32_t g_firstInstance{ 0U };
constexpr uint32_t g_firstIndex{ 0U };
constexpr vk::DeviceSize g_offset{ 0UL };
constexpr uint32_t g_firstBinding{ 0U };
constexpr uint32_t g_firstScissor{ 0U };
constexpr uint32_t g_firstViewport{ 0U };
constexpr vk::ClearColorValue g_clearColor{ 0.1F, 0.1F, 0.1F, 1.0F };
constexpr vk::ClearDepthStencilValue g_clearDepthStencil{ 1.0F, 0U };
constexpr std::array< vk::ClearValue, 2U > g_clearValues{ g_clearColor, g_clearDepthStencil };
} // namespace

namespace ve {

uint32_t GraphicsCommandBuffer::getQueueFamilyID( const ve::LogicalDevice& logicalDevice ) {
    return logicalDevice.getQueueFamilyIDs().at( ve::FamilyType::eGraphics );
}

void GraphicsCommandBuffer::beginRenderPass( const vk::RenderPass renderPass, const vk::Framebuffer framebuffer,
                                             const vk::Extent2D renderArea ) const noexcept {
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType             = vk::StructureType::eRenderPassBeginInfo;
    renderPassBeginInfo.renderPass        = renderPass;
    renderPassBeginInfo.framebuffer       = framebuffer;
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassBeginInfo.renderArea.extent = renderArea;

    renderPassBeginInfo.clearValueCount = static_cast< uint32_t >( std::size( g_clearValues ) );
    renderPassBeginInfo.pClearValues    = std::data( g_clearValues );

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
                                               const uint32_t firstSet ) const noexcept {
    m_commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayout, firstSet, descriptorSet,
                                        nullptr );
}

void GraphicsCommandBuffer::draw( const uint32_t verticesCount ) const noexcept {
    m_commandBuffer.draw( verticesCount, g_instanceCount, g_firstVertex, g_firstInstance );
}

void GraphicsCommandBuffer::drawIndices( const uint32_t firstIndex, const uint32_t indicesCount ) const noexcept {
    m_commandBuffer.drawIndexed( indicesCount, g_instanceCount, firstIndex, g_offset, g_firstInstance );
}

void GraphicsCommandBuffer::endRenderPass() const noexcept {
    m_commandBuffer.endRenderPass();
}

void GraphicsCommandBuffer::transitionImageBuffer( const vk::Image image, const vk::Format format,
                                                   const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout ) {
    vk::ImageMemoryBarrier barrier{};
    barrier.sType               = vk::StructureType::eImageMemoryBarrier;
    barrier.oldLayout           = oldLayout;
    barrier.newLayout           = newLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image               = image;

    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel   = 0U;
    barrier.subresourceRange.levelCount     = 1U;
    barrier.subresourceRange.baseArrayLayer = 0U;
    barrier.subresourceRange.layerCount     = 1U;

    vk::PipelineStageFlagBits srcStage{};
    vk::PipelineStageFlagBits dstStage{};
    if ( oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal ) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        srcStage              = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage              = vk::PipelineStageFlagBits::eTransfer;
    } else if ( oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal ) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage              = vk::PipelineStageFlagBits::eTransfer;
        dstStage              = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument( "unsupported layout transition" );
    }

    static constexpr vk::DependencyFlags flags{};
    m_commandBuffer.pipelineBarrier( srcStage, dstStage, flags, nullptr, nullptr, barrier );
}

void GraphicsCommandBuffer::copyBufferToImage( const vk::Buffer buffer, const vk::Image image,
                                               const vk::Extent2D extent ) {
    vk::BufferImageCopy copyRegion{};
    copyRegion.bufferOffset      = 0U;
    copyRegion.bufferRowLength   = 0U;
    copyRegion.bufferImageHeight = 0U;

    copyRegion.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    copyRegion.imageSubresource.mipLevel       = 0U;
    copyRegion.imageSubresource.baseArrayLayer = 0U;
    copyRegion.imageSubresource.layerCount     = 1U;

    copyRegion.imageOffset = vk::Offset3D{};
    copyRegion.imageExtent = vk::Extent3D{ extent.width, extent.height, 1U };

    m_commandBuffer.copyBufferToImage( buffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegion );
}

void GraphicsCommandBuffer::pushConstants( const vk::PipelineLayout layout, const vk::ShaderStageFlags shaderStages,
                                           const ve::PushConstants& pushConstants,
                                           const uint32_t offset ) const noexcept {
    m_commandBuffer.pushConstants( layout, shaderStages, offset, sizeof( ve::PushConstants ), &pushConstants );
}

} // namespace ve
