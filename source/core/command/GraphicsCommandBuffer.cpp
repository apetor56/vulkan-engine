#include "GraphicsCommandBuffer.hpp"
#include "QueueFamilyIDs.hpp"
#include "LogicalDevice.hpp"
#include "Mesh.hpp"

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

void GraphicsCommandBuffer::drawVertices( const uint32_t firstVertex, const uint32_t vertexCount ) const noexcept {
    m_commandBuffer.draw( vertexCount, g_instanceCount, g_firstIndex, g_firstInstance );
}

void GraphicsCommandBuffer::drawIndices( const uint32_t firstIndex, const uint32_t indicesCount ) const noexcept {
    m_commandBuffer.drawIndexed( indicesCount, g_instanceCount, firstIndex, g_offset, g_firstInstance );
}

void GraphicsCommandBuffer::transitionImageLayout( const vk::Image image, [[maybe_unused]] const vk::Format format,
                                                   const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout,
                                                   const uint32_t mipLevel, const uint32_t layerCount ) const {
    vk::ImageMemoryBarrier barrier{};
    barrier.sType               = vk::StructureType::eImageMemoryBarrier;
    barrier.oldLayout           = oldLayout;
    barrier.newLayout           = newLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image               = image;

    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel   = 0U;
    barrier.subresourceRange.levelCount     = mipLevel;
    barrier.subresourceRange.baseArrayLayer = 0U;
    barrier.subresourceRange.layerCount     = layerCount;

    vk::PipelineStageFlagBits sourceStage{};
    vk::PipelineStageFlagBits destinationStage{};
    if ( oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal ) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage           = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage      = vk::PipelineStageFlagBits::eTransfer;
    } else if ( oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal ) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage           = vk::PipelineStageFlagBits::eTransfer;
        destinationStage      = vk::PipelineStageFlagBits::eFragmentShader;
    }

    else if ( oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal ) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        sourceStage           = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage      = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if ( oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
                newLayout == vk::ImageLayout::ePresentSrcKHR ) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eNone;
        sourceStage           = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        destinationStage      = vk::PipelineStageFlagBits::eBottomOfPipe;
    } else if ( oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal ) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage           = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        destinationStage      = vk::PipelineStageFlagBits::eFragmentShader;
    }

    else {
        throw std::invalid_argument( "unsupported layout transition" );
    }

    static constexpr vk::DependencyFlags flags{};
    m_commandBuffer.pipelineBarrier( sourceStage, destinationStage, flags, nullptr, nullptr, barrier );
}

void GraphicsCommandBuffer::copyBufferToImage( const vk::Buffer buffer, const vk::Image image,
                                               const vk::Extent2D extent, const uint32_t layerCount ) {
    vk::BufferImageCopy copyRegion{};
    copyRegion.bufferOffset      = 0U;
    copyRegion.bufferRowLength   = 0U;
    copyRegion.bufferImageHeight = 0U;

    copyRegion.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    copyRegion.imageSubresource.mipLevel       = 0U;
    copyRegion.imageSubresource.baseArrayLayer = 0U;
    copyRegion.imageSubresource.layerCount     = layerCount;

    copyRegion.imageOffset = vk::Offset3D{};
    copyRegion.imageExtent = vk::Extent3D{ extent.width, extent.height, 1U };

    m_commandBuffer.copyBufferToImage( buffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegion );
}

void GraphicsCommandBuffer::pushConstants( const vk::PipelineLayout layout, const vk::ShaderStageFlags shaderStages,
                                           const ve::PushConstants& pushConstants,
                                           const uint32_t offset ) const noexcept {
    m_commandBuffer.pushConstants( layout, shaderStages, offset, sizeof( ve::PushConstants ), &pushConstants );
}

void GraphicsCommandBuffer::beginRendering( const vk::Extent2D extent, const vk::ImageView sampledImageView,
                                            const vk::ImageView resolvedImageView,
                                            const vk::ImageView depthView ) const {
    vk::RenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.pNext              = nullptr;
    colorAttachment.imageView          = sampledImageView;
    colorAttachment.imageLayout        = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.resolveImageView   = resolvedImageView;
    colorAttachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.resolveMode        = vk::ResolveModeFlagBits::eAverage;
    colorAttachment.loadOp             = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp            = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue         = g_clearColor;

    vk::RenderingAttachmentInfoKHR depthAttachment{};
    depthAttachment.pNext       = nullptr;
    depthAttachment.imageView   = depthView;
    depthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthAttachment.loadOp      = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp     = vk::AttachmentStoreOp::eStore;
    depthAttachment.clearValue  = g_clearDepthStencil;

    static constexpr vk::Offset2D defaultOffset{ 0, 0 };
    vk::RenderingInfoKHR renderingInfo{};
    renderingInfo.layerCount           = 1U;
    renderingInfo.colorAttachmentCount = 1U;
    renderingInfo.renderArea           = vk::Rect2D{ defaultOffset, extent };
    renderingInfo.pColorAttachments    = &colorAttachment;
    renderingInfo.pDepthAttachment     = &depthAttachment;

    m_commandBuffer.beginRendering( renderingInfo );
}

void GraphicsCommandBuffer::endRendering() const {
    m_commandBuffer.endRendering();
}

} // namespace ve
