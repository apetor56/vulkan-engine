#include "TransferCommandBuffer.hpp"
#include "LogicalDevice.hpp"

namespace ve {

uint32_t TransferCommandBuffer::getQueueFamilyID( const ve::LogicalDevice& logicalDevice ) {
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

} // namespace ve
