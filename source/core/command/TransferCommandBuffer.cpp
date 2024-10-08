#include "TransferCommandBuffer.hpp"
#include "LogicalDevice.hpp"

namespace ve {

std::uint32_t TransferCommandBuffer::getQueueFamilyID( const ve::LogicalDevice& logicalDevice ) {
    return logicalDevice.getQueueFamilyIDs().at( ve::FamilyType::eTransfer );
}

void TransferCommandBuffer::copyBuffer( const vk::DeviceSize srcOffset, const vk::DeviceSize dstOffset,
                                        const std::size_t size, const vk::Buffer srcBuffer,
                                        const vk::Buffer dstBuffer ) const {
    vk::BufferCopy copyRegion;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size      = size;
    m_commandBuffer.copyBuffer( srcBuffer, dstBuffer, copyRegion );
}

void TransferCommandBuffer::transitionImageBuffer( const vk::Image image, const vk::Format format,
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

void TransferCommandBuffer::copyBufferToImage( const vk::Buffer buffer, const vk::Image image,
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

} // namespace ve
