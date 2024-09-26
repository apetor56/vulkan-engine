#pragma once

#include "LogicalDevice.hpp"
#include "GraphicsCommandBuffer.hpp"
#include "TransferCommandBuffer.hpp"

namespace ve {

template < std::derived_from< BaseCommandBuffer > CommandBuffer_T >

class CommandPool {
public:
    CommandPool( const ve::LogicalDevice& logicalDevice ) : m_logicalDevice{ logicalDevice } { createCommandPool(); }
    ~CommandPool() { m_logicalDevice.getHandler().destroyCommandPool( m_commandPool ); }

    CommandPool( const CommandPool& other ) = delete;
    CommandPool( CommandPool&& other )      = delete;

    CommandPool& operator=( const CommandPool& other ) = delete;
    CommandPool& operator=( CommandPool&& other )      = delete;

    template < vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary >
    std::vector< CommandBuffer_T > createCommandBuffers( const std::uint32_t count ) {
        const auto allocInfo{ createAllocInfo( count, level ) };
        const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
        const auto commandBufferHandlers{ logicalDeviceHandler.allocateCommandBuffers( allocInfo ) };

        std::vector< CommandBuffer_T > m_commandBuffers;
        for ( std::uint32_t index{ 0 }; index < count; index++ )
            m_commandBuffers.emplace_back( commandBufferHandlers.at( index ) );

        return m_commandBuffers;
    }

    void freeCommandBuffer( const CommandBuffer_T commandBuffer ) const {
        m_logicalDevice.getHandler().freeCommandBuffers( m_commandPool, commandBuffer.getHandler() );
    }

private:
    vk::CommandPool m_commandPool;
    const ve::LogicalDevice& m_logicalDevice;

    void createCommandPool() {
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = vk::StructureType::eCommandPoolCreateInfo;
        poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = CommandBuffer_T::getQueueFamilyID( m_logicalDevice );

        m_commandPool = m_logicalDevice.getHandler().createCommandPool( poolInfo );
    }

    vk::CommandBufferAllocateInfo createAllocInfo( const std::uint32_t buffersCount,
                                                   const vk::CommandBufferLevel level ) const noexcept {
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.commandPool        = m_commandPool;
        allocInfo.level              = level;
        allocInfo.commandBufferCount = buffersCount;

        return allocInfo;
    }
};

} // namespace ve
