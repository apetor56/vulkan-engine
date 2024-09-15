#pragma once

#include "Vertex.hpp"
#include "LogicalDevice.hpp"
#include "command/CommandPool.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

template < typename T >
concept BufferStorageType = std::is_same_v< T, ve::Vertex > || std::is_integral_v< T >;

namespace ve {

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
class Buffer {
public:
    Buffer( const ve::LogicalDevice& logicalDevice, std::vector< T > data );
    ~Buffer();

    Buffer( const Buffer& other ) = delete;
    Buffer( Buffer&& other )      = delete;

    Buffer& operator=( const Buffer& other ) = delete;
    Buffer& operator=( Buffer&& other )      = delete;

    vk::Buffer getHandler() const noexcept;
    std::uint32_t getCount() const noexcept;

private:
    std::vector< T > m_data{};
    vk::Buffer m_buffer{};
    vk::DeviceMemory m_bufferMemory{};
    const ve::LogicalDevice& m_logicalDevice;
    inline static constexpr std::uint32_t s_bufferOffset{ 0U };

    vk::Buffer createBuffer( const vk::BufferUsageFlags usage ) const;
    vk::DeviceMemory allocateMemory( const vk::Buffer buffer, const vk::MemoryPropertyFlags memoryProperties ) const;
    void bindBufferMemory( const vk::Buffer buffer, const vk::DeviceMemory memory ) const;
    void setData( const vk::DeviceMemory memory ) const;
    std::uint32_t getMemoryTypeIndex( const std::uint32_t bufferMemoryTypeBits,
                                      const vk::MemoryPropertyFlags wantedPropertiesFlag ) const;
};

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
Buffer< bufferUsage, T >::Buffer( const ve::LogicalDevice& logicalDevice, std::vector< T > data )
    : m_data{ std::move( data ) }, m_logicalDevice{ logicalDevice } {
    const auto stagingBuffer{ createBuffer( vk::BufferUsageFlagBits::eTransferSrc ) };
    const auto stagingBufferMemory{ allocateMemory( stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible |
                                                                       vk::MemoryPropertyFlagBits::eHostCoherent ) };
    bindBufferMemory( stagingBuffer, stagingBufferMemory );
    setData( stagingBufferMemory );

    m_buffer       = createBuffer( vk::BufferUsageFlagBits::eTransferDst | bufferUsage );
    m_bufferMemory = allocateMemory( m_buffer, vk::MemoryPropertyFlagBits::eDeviceLocal );
    bindBufferMemory( m_buffer, m_bufferMemory );

    ve::CommandPool< ve::FamilyType::eTransfer > transferCommandPool{ m_logicalDevice };
    const auto commandBuffer{ std::move( transferCommandPool.createCommandBuffers( 1U ).at( 0 ) ) };
    commandBuffer.begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
    commandBuffer.copyBuffer( stagingBuffer, m_buffer, sizeof( T ) * std::size( m_data ) );
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

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
Buffer< bufferUsage, T >::~Buffer() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    logicalDeviceHandler.destroyBuffer( m_buffer );
    logicalDeviceHandler.freeMemory( m_bufferMemory );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
vk::Buffer Buffer< bufferUsage, T >::createBuffer( const vk::BufferUsageFlags usage ) const {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto allQueueFamiles{ m_logicalDevice.getQueueFamilyIDs() };
    const std::array< std::uint32_t, 2U > usedFamilyIDs{ allQueueFamiles.at( FamilyType::eGraphics ),
                                                         allQueueFamiles.at( FamilyType::eTransfer ) };

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.sType                 = vk::StructureType::eBufferCreateInfo;
    bufferInfo.size                  = sizeof( T ) * std::size( m_data );
    bufferInfo.usage                 = usage;
    bufferInfo.sharingMode           = vk::SharingMode::eConcurrent;
    bufferInfo.queueFamilyIndexCount = static_cast< std::uint32_t >( std::size( usedFamilyIDs ) );
    bufferInfo.pQueueFamilyIndices   = std::data( usedFamilyIDs );

    return logicalDeviceHandler.createBuffer( bufferInfo );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
vk::DeviceMemory Buffer< bufferUsage, T >::allocateMemory( const vk::Buffer buffer,
                                                           const vk::MemoryPropertyFlags memoryProperties ) const {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto bufferMemoryRequirements{ logicalDeviceHandler.getBufferMemoryRequirements( buffer ) };

    vk::MemoryAllocateInfo allocationInfo{};
    allocationInfo.sType           = vk::StructureType::eMemoryAllocateInfo;
    allocationInfo.allocationSize  = bufferMemoryRequirements.size;
    allocationInfo.memoryTypeIndex = getMemoryTypeIndex( bufferMemoryRequirements.memoryTypeBits, memoryProperties );

    return logicalDeviceHandler.allocateMemory( allocationInfo );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
void Buffer< bufferUsage, T >::bindBufferMemory( const vk::Buffer buffer, const vk::DeviceMemory memory ) const {
    m_logicalDevice.getHandler().bindBufferMemory( buffer, memory, s_bufferOffset );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
void Buffer< bufferUsage, T >::setData( const vk::DeviceMemory memory ) const {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

    const auto bufferSize{ sizeof( T ) * std::size( m_data ) };
    void *cpuMemory{ logicalDeviceHandler.mapMemory( memory, s_bufferOffset, bufferSize ) };
    memcpy( cpuMemory, std::data( m_data ), sizeof( T ) * std::size( m_data ) );
    logicalDeviceHandler.unmapMemory( memory );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
std::uint32_t Buffer< bufferUsage, T >::getMemoryTypeIndex( const std::uint32_t bufferMemoryTypeBits,
                                                            const vk::MemoryPropertyFlags wantedPropertiesFlag ) const {
    const auto deviceMemoryProperties{ m_logicalDevice.getMemoryProperties() };

    const auto isDeviceMemorySuitableForBuffer{ [ bufferMemoryTypeBits ]( auto deviceMemoryTypeBit ) {
        return ( bufferMemoryTypeBits & deviceMemoryTypeBit ) != 0;
    } };
    const auto arePropertiesSupported{ [ &deviceMemoryProperties, wantedPropertiesFlag ]( std::uint32_t memoryTypeID ) {
        return ( deviceMemoryProperties.memoryTypes.at( memoryTypeID ).propertyFlags & wantedPropertiesFlag ) ==
               wantedPropertiesFlag;
    } };

    for ( std::uint32_t memoryTypeID{ 0U }; memoryTypeID < deviceMemoryProperties.memoryTypeCount; memoryTypeID++ ) {
        if ( isDeviceMemorySuitableForBuffer( 1 << memoryTypeID ) && arePropertiesSupported( memoryTypeID ) )
            return memoryTypeID;
    }

    throw std::runtime_error( "failed to find memory type suitable for vertex buffer" );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
vk::Buffer Buffer< bufferUsage, T >::getHandler() const noexcept {
    return m_buffer;
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
std::uint32_t Buffer< bufferUsage, T >::getCount() const noexcept {
    return static_cast< std::uint32_t >( std::size( m_data ) );
}

using VertexBuffer = Buffer< vk::BufferUsageFlagBits::eVertexBuffer, Vertex >;

} // namespace ve
