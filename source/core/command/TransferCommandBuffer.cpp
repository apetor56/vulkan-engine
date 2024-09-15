#include "TransferCommandBuffer.hpp"

namespace ve {

void TransferCommandBuffer::copyBuffer( const vk::Buffer stagingBuffer, const vk::Buffer actualBuffer,
                                        const std::size_t size ) const {
    vk::BufferCopy copyRegion;
    copyRegion.size = size;
    m_commandBuffer.copyBuffer( stagingBuffer, actualBuffer, copyRegion );
}

} // namespace ve
