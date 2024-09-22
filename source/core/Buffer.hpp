#pragma once

#include "Vertex.hpp"
#include "LogicalDevice.hpp"
#include "command/CommandPool.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <vector>

struct UniformBufferData {
    glm::mat4 model{ 1.0F };
    glm::mat4 view{ 1.0F };
    glm::mat4 projection{ 1.0F };

    constexpr auto size() const { return 1U; }
    auto data() const { return this; }
};

template < typename T >
concept BufferData = requires( T data ) {
    data.size();
    data.data();
};

namespace ve {

template < vk::BufferUsageFlagBits bufferUsage, BufferData T >
class Buffer {
public:
    Buffer( const ve::LogicalDevice& logicalDevice, T data )
        : m_logicalDevice{ logicalDevice }, m_dataCount{ static_cast< std::uint32_t >( std::size( data ) ) } {
        const auto stagingBuffer{ createBuffer( vk::BufferUsageFlagBits::eTransferSrc ) };
        const auto stagingBufferMemory{ allocateMemory(
            stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent ) };
        bindBufferMemory( stagingBuffer, stagingBufferMemory );
        mapMemory( stagingBufferMemory );
        setData( data );
        unmapMemory( stagingBufferMemory );

        m_buffer       = createBuffer( vk::BufferUsageFlagBits::eTransferDst | bufferUsage );
        m_bufferMemory = allocateMemory( m_buffer, vk::MemoryPropertyFlagBits::eDeviceLocal );
        bindBufferMemory( m_buffer, m_bufferMemory );

        ve::CommandPool< ve::FamilyType::eTransfer > transferCommandPool{ m_logicalDevice };
        const auto commandBuffer{ std::move( transferCommandPool.createCommandBuffers( 1U ).at( 0 ) ) };
        commandBuffer.begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
        commandBuffer.copyBuffer( stagingBuffer, m_buffer, sizeof( T ) * m_dataCount );
        commandBuffer.end();

        const auto commandBufferHandler{ commandBuffer.getHandler() };
        vk::SubmitInfo submitInfo;
        submitInfo.sType              = vk::StructureType::eSubmitInfo;
        submitInfo.commandBufferCount = 1U;
        submitInfo.pCommandBuffers    = &commandBufferHandler;

        const auto transferQueue{ m_logicalDevice.getQueue( QueueType::eTransfer ) };
        transferQueue.submit( submitInfo );
        transferQueue.waitIdle();

        const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
        logicalDeviceHandler.destroyBuffer( stagingBuffer );
        logicalDeviceHandler.freeMemory( stagingBufferMemory );
        transferCommandPool.freeCommandBuffer( commandBufferHandler );
    }

    ~Buffer() {
        const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
        logicalDeviceHandler.destroyBuffer( m_buffer );
        logicalDeviceHandler.freeMemory( m_bufferMemory );
    }

    Buffer( const Buffer& other ) = delete;
    Buffer( Buffer&& other )      = delete;

    Buffer& operator=( const Buffer& other ) = delete;
    Buffer& operator=( Buffer&& other )      = delete;

    vk::Buffer getHandler() const noexcept { return m_buffer; }
    std::uint32_t getCount() const noexcept { return m_dataCount; }
    void *getMapperMemory() const noexcept { return m_cpuMemory; }

private:
    vk::Buffer m_buffer{};
    vk::DeviceMemory m_bufferMemory{};
    const ve::LogicalDevice& m_logicalDevice;
    void *m_cpuMemory;
    inline static constexpr std::uint32_t s_bufferOffset{ 0U };
    std::uint32_t m_dataCount;

    vk::Buffer createBuffer( const vk::BufferUsageFlags usage ) const {
        const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
        const auto allQueueFamiles{ m_logicalDevice.getQueueFamilyIDs() };
        const std::array< std::uint32_t, 2U > usedFamilyIDs{ allQueueFamiles.at( FamilyType::eGraphics ),
                                                             allQueueFamiles.at( FamilyType::eTransfer ) };

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.sType                 = vk::StructureType::eBufferCreateInfo;
        bufferInfo.size                  = sizeof( T ) * m_dataCount;
        bufferInfo.usage                 = usage;
        bufferInfo.sharingMode           = vk::SharingMode::eConcurrent;
        bufferInfo.queueFamilyIndexCount = static_cast< std::uint32_t >( std::size( usedFamilyIDs ) );
        bufferInfo.pQueueFamilyIndices   = std::data( usedFamilyIDs );

        return logicalDeviceHandler.createBuffer( bufferInfo );
    }

    vk::DeviceMemory allocateMemory( const vk::Buffer buffer, const vk::MemoryPropertyFlags memoryProperties ) const {
        const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
        const auto bufferMemoryRequirements{ logicalDeviceHandler.getBufferMemoryRequirements( buffer ) };

        vk::MemoryAllocateInfo allocationInfo{};
        allocationInfo.sType          = vk::StructureType::eMemoryAllocateInfo;
        allocationInfo.allocationSize = bufferMemoryRequirements.size;
        allocationInfo.memoryTypeIndex =
            getMemoryTypeIndex( bufferMemoryRequirements.memoryTypeBits, memoryProperties );

        return logicalDeviceHandler.allocateMemory( allocationInfo );
    }

    void bindBufferMemory( const vk::Buffer buffer, const vk::DeviceMemory memory ) const {
        m_logicalDevice.getHandler().bindBufferMemory( buffer, memory, s_bufferOffset );
    }

    void mapMemory( const vk::DeviceMemory memory ) {
        const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
        const auto bufferSize{ sizeof( T ) * m_dataCount };
        m_cpuMemory = logicalDeviceHandler.mapMemory( memory, s_bufferOffset, bufferSize );
    }

    void setData( const T& data ) const { memcpy( m_cpuMemory, std::data( data ), sizeof( T ) * std::size( data ) ); }

    void unmapMemory( const vk::DeviceMemory memory ) const { m_logicalDevice.getHandler().unmapMemory( memory ); }

    std::uint32_t getMemoryTypeIndex( const std::uint32_t bufferMemoryTypeBits,
                                      const vk::MemoryPropertyFlags wantedPropertiesFlag ) const {
        const auto deviceMemoryProperties{ m_logicalDevice.getMemoryProperties() };

        const auto isDeviceMemorySuitableForBuffer{ [ bufferMemoryTypeBits ]( auto deviceMemoryTypeBit ) {
            return ( bufferMemoryTypeBits & deviceMemoryTypeBit ) != 0;
        } };
        const auto arePropertiesSupported{
            [ &deviceMemoryProperties, wantedPropertiesFlag ]( std::uint32_t memoryTypeID ) {
                return ( deviceMemoryProperties.memoryTypes.at( memoryTypeID ).propertyFlags & wantedPropertiesFlag ) ==
                       wantedPropertiesFlag;
            } };

        for ( std::uint32_t memoryTypeID{ 0U }; memoryTypeID < deviceMemoryProperties.memoryTypeCount;
              memoryTypeID++ ) {
            if ( isDeviceMemorySuitableForBuffer( 1 << memoryTypeID ) && arePropertiesSupported( memoryTypeID ) )
                return memoryTypeID;
        }

        throw std::runtime_error( "failed to find memory type suitable for vertex buffer" );
    }
};

template <>
inline Buffer< vk::BufferUsageFlagBits::eUniformBuffer, UniformBufferData >::Buffer(
    const ve::LogicalDevice& logicalDevice, UniformBufferData data )
    : m_logicalDevice{ logicalDevice }, m_dataCount{ static_cast< std::uint32_t >( std::size( data ) ) } {
    m_buffer       = createBuffer( vk::BufferUsageFlagBits::eUniformBuffer );
    m_bufferMemory = allocateMemory( m_buffer, vk::MemoryPropertyFlagBits::eHostVisible |
                                                   vk::MemoryPropertyFlagBits::eHostCoherent );
    bindBufferMemory( m_buffer, m_bufferMemory );
    mapMemory( m_bufferMemory );
}

using VertexBuffer  = Buffer< vk::BufferUsageFlagBits::eVertexBuffer, std::vector< Vertex > >;
using IndexBuffer   = Buffer< vk::BufferUsageFlagBits::eIndexBuffer, std::vector< std::uint32_t > >;
using UniformBuffer = Buffer< vk::BufferUsageFlagBits::eUniformBuffer, UniformBufferData >;

} // namespace ve
