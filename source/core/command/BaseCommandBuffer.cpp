#include "BaseCommandBuffer.hpp"

namespace ve {

BaseCommandBuffer::BaseCommandBuffer( const vk::CommandBuffer commandBufferHandler ) noexcept
    : m_commandBuffer{ commandBufferHandler } {}

void BaseCommandBuffer::begin( const vk::CommandBufferUsageFlags flags ) const {
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
    beginInfo.flags = flags;

    m_commandBuffer.begin( beginInfo );
}

void BaseCommandBuffer::end() const {
    m_commandBuffer.end();
}

void BaseCommandBuffer::reset() const {
    m_commandBuffer.reset();
}

} // namespace ve
