#include "TransferCommandBuffer.hpp"
#include "LogicalDevice.hpp"

namespace ve {

std::uint32_t TransferCommandBuffer::getQueueFamilyID( const ve::LogicalDevice& logicalDevice ) {
    return logicalDevice.getQueueFamilyIDs().at( ve::FamilyType::eTransfer );
}

void TransferCommandBuffer::copyBuffer( const vk::Buffer stagingBuffer, const vk::Buffer actualBuffer,
                                        const std::size_t size ) const {
    vk::BufferCopy copyRegion;
    copyRegion.size = size;
    m_commandBuffer.copyBuffer( stagingBuffer, actualBuffer, copyRegion );
}

} // namespace ve
