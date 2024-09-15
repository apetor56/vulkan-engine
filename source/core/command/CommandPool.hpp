#pragma once

#include "LogicalDevice.hpp"
#include "GraphicsCommandBuffer.hpp"
#include "TransferCommandBuffer.hpp"

namespace ve {

template < ve::FamilyType type >
class CommandPool {
public:
    CommandPool( const ve::LogicalDevice& logicalDevice ) : m_logicalDevice{ logicalDevice } { createCommandPool(); }
    ~CommandPool() { m_logicalDevice.getHandler().destroyCommandPool( m_commandPool ); }

    CommandPool( const CommandPool& other ) = delete;
    CommandPool( CommandPool&& other )      = delete;

    CommandPool& operator=( const CommandPool& other ) = delete;
    CommandPool& operator=( CommandPool&& other )      = delete;

    auto createCommandBuffers( const std::uint32_t count );
    void freeCommandBuffer( const vk::CommandBuffer commandBuffer ) const;

private:
    vk::CommandPool m_commandPool;
    const ve::LogicalDevice& m_logicalDevice;

    void createCommandPool();
    vk::CommandBufferAllocateInfo createAllocInfo( const std::uint32_t buffersCount ) const noexcept;
};

template < ve::FamilyType type >
void CommandPool< type >::createCommandPool() {
    const auto queueFamilies{ m_logicalDevice.getQueueFamilyIDs() };

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = vk::StructureType::eCommandPoolCreateInfo;
    poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueFamilies.at( type );

    m_commandPool = m_logicalDevice.getHandler().createCommandPool( poolInfo );
}

template <>
inline auto CommandPool< ve::FamilyType::eGraphics >::createCommandBuffers( const std::uint32_t count ) {
    const auto allocInfo{ createAllocInfo( count ) };
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto commandBufferHandlers{ logicalDeviceHandler.allocateCommandBuffers( allocInfo ) };

    std::vector< ve::GraphicsCommandBuffer > m_commandBuffers;
    for ( std::uint32_t index{ 0 }; index < count; index++ )
        m_commandBuffers.emplace_back( commandBufferHandlers.at( index ) );

    return m_commandBuffers;
}

template <>
inline auto CommandPool< ve::FamilyType::eTransfer >::createCommandBuffers( const std::uint32_t count ) {
    const auto allocInfo{ createAllocInfo( count ) };
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto commandBufferHandlers{ logicalDeviceHandler.allocateCommandBuffers( allocInfo ) };

    std::vector< ve::TransferCommandBuffer > m_commandBuffers;
    for ( std::uint32_t index{ 0 }; index < count; index++ )
        m_commandBuffers.emplace_back( commandBufferHandlers.at( index ) );

    return m_commandBuffers;
}

template < ve::FamilyType type >
vk::CommandBufferAllocateInfo CommandPool< type >::createAllocInfo( const std::uint32_t buffersCount ) const noexcept {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = buffersCount;

    return allocInfo;
}

template < ve::FamilyType type >
void CommandPool< type >::freeCommandBuffer( const vk::CommandBuffer commandBuffer ) const {
    m_logicalDevice.getHandler().freeCommandBuffers( m_commandPool, commandBuffer );
}

} // namespace ve
