#pragma once

#include "LogicalDevice.hpp"
#include "GraphicsCommandBuffer.hpp"
#include "TransferCommandBuffer.hpp"

namespace ve {

template < std::derived_from< BaseCommandBuffer > CommandBuffer_T >
class CommandPool : public utils::NonCopyable,
                    public utils::NonMovable {
public:
    CommandPool( const ve::LogicalDevice& logicalDevice ) : m_logicalDevice{ logicalDevice } { createCommandPool(); }
    ~CommandPool() { m_logicalDevice.get().destroyCommandPool( m_commandPool ); }

    template < std::uint32_t count = 1U, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary >
    auto createCommandBuffers() const {
        const auto allocInfo{ createAllocInfo( count, level ) };
        const auto logicalDeviceHandler{ m_logicalDevice.get() };
        const auto commandBufferHandlers{ logicalDeviceHandler.allocateCommandBuffers( allocInfo ) };

        if constexpr ( count == 1U ) {
            return CommandBuffer_T{ *commandBufferHandlers.begin() };
        } else {
            std::vector< CommandBuffer_T > m_commandBuffers;
            for ( std::uint32_t index{ 0 }; index < count; index++ )
                m_commandBuffers.emplace_back( commandBufferHandlers.at( index ) );

            return m_commandBuffers;
        }
    }

    void freeCommandBuffer( const CommandBuffer_T commandBuffer ) const {
        m_logicalDevice.get().freeCommandBuffers( m_commandPool, commandBuffer.get() );
    }

private:
    vk::CommandPool m_commandPool;
    const ve::LogicalDevice& m_logicalDevice;

    void createCommandPool() {
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = vk::StructureType::eCommandPoolCreateInfo;
        poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = CommandBuffer_T::getQueueFamilyID( m_logicalDevice );

        m_commandPool = m_logicalDevice.get().createCommandPool( poolInfo );
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
