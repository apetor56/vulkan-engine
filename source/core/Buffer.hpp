#pragma once

#include "Vertex.hpp"
#include "LogicalDevice.hpp"

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

    void createBuffer();
    void allocateMemory();
    void bindBufferMemory();
    void setData();
    std::uint32_t getMemoryTypeIndex( const std::uint32_t bufferMemoryTypeBits,
                                      vk::MemoryPropertyFlags wantedPropertiesFlag ) const;
};

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
Buffer< bufferUsage, T >::Buffer( const ve::LogicalDevice& logicalDevice, std::vector< T > data )
    : m_logicalDevice{ logicalDevice }, m_data{ std::move( data ) } {
    createBuffer();
    allocateMemory();
    bindBufferMemory();
    setData();
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
Buffer< bufferUsage, T >::~Buffer() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    logicalDeviceHandler.destroyBuffer( m_buffer );
    logicalDeviceHandler.freeMemory( m_bufferMemory );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
void Buffer< bufferUsage, T >::createBuffer() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.sType       = vk::StructureType::eBufferCreateInfo;
    bufferInfo.size        = sizeof( T ) * std::size( m_data );
    bufferInfo.usage       = bufferUsage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    m_buffer = logicalDeviceHandler.createBuffer( bufferInfo );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
void Buffer< bufferUsage, T >::allocateMemory() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto bufferMemoryRequirements{ logicalDeviceHandler.getBufferMemoryRequirements( m_buffer ) };

    vk::MemoryAllocateInfo allocationInfo{};
    static constexpr auto properties{ vk::MemoryPropertyFlagBits::eHostVisible |
                                      vk::MemoryPropertyFlagBits::eHostCoherent };

    allocationInfo.sType           = vk::StructureType::eMemoryAllocateInfo;
    allocationInfo.allocationSize  = bufferMemoryRequirements.size;
    allocationInfo.memoryTypeIndex = getMemoryTypeIndex( bufferMemoryRequirements.memoryTypeBits, properties );

    m_bufferMemory = logicalDeviceHandler.allocateMemory( allocationInfo );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
void Buffer< bufferUsage, T >::bindBufferMemory() {
    m_logicalDevice.getHandler().bindBufferMemory( m_buffer, m_bufferMemory, s_bufferOffset );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
void Buffer< bufferUsage, T >::setData() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

    const auto bufferSize{ sizeof( T ) * std::size( m_data ) };
    void *cpuMemory{ logicalDeviceHandler.mapMemory( m_bufferMemory, s_bufferOffset, bufferSize ) };
    memcpy( cpuMemory, std::data( m_data ), sizeof( T ) * std::size( m_data ) );
    logicalDeviceHandler.unmapMemory( m_bufferMemory );
}

template < vk::BufferUsageFlagBits bufferUsage, BufferStorageType T >
std::uint32_t Buffer< bufferUsage, T >::getMemoryTypeIndex( const std::uint32_t bufferMemoryTypeBits,
                                                            vk::MemoryPropertyFlags wantedPropertiesFlag ) const {
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
