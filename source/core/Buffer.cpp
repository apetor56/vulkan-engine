#include "Buffer.hpp"

namespace ve {

Buffer::Buffer( const ve::LogicalDevice& logicalDevice ) : m_logicalDevice{ logicalDevice } {
    createBuffer();
    allocateMemory();
    bindBufferMemory();
    setData();
}

Buffer::~Buffer() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    logicalDeviceHandler.destroyBuffer( m_vertexBuffer );
    logicalDeviceHandler.freeMemory( m_vertexBufferMemory );
}

void Buffer::createBuffer() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.sType       = vk::StructureType::eBufferCreateInfo;
    bufferInfo.size        = sizeof( Vertex ) * std::size( m_vertices );
    bufferInfo.usage       = vk::BufferUsageFlagBits::eVertexBuffer;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    m_vertexBuffer = logicalDeviceHandler.createBuffer( bufferInfo );
}

void Buffer::allocateMemory() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto bufferMemoryRequirements{ logicalDeviceHandler.getBufferMemoryRequirements( m_vertexBuffer ) };

    vk::MemoryAllocateInfo allocationInfo{};
    static constexpr auto properties{ vk::MemoryPropertyFlagBits::eHostVisible |
                                      vk::MemoryPropertyFlagBits::eHostCoherent };

    allocationInfo.sType           = vk::StructureType::eMemoryAllocateInfo;
    allocationInfo.allocationSize  = bufferMemoryRequirements.size;
    allocationInfo.memoryTypeIndex = getMemoryTypeIndex( bufferMemoryRequirements.memoryTypeBits, properties );

    m_vertexBufferMemory = logicalDeviceHandler.allocateMemory( allocationInfo );
}

void Buffer::bindBufferMemory() {
    m_logicalDevice.getHandler().bindBufferMemory( m_vertexBuffer, m_vertexBufferMemory, s_bufferOffset );
}

void Buffer::setData() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

    const auto bufferSize{ sizeof( Vertex ) * std::size( m_vertices ) };
    void *cpuMemory{ logicalDeviceHandler.mapMemory( m_vertexBufferMemory, s_bufferOffset, bufferSize ) };
    memcpy( cpuMemory, std::data( m_vertices ), sizeof( Vertex ) * std::size( m_vertices ) );
    logicalDeviceHandler.unmapMemory( m_vertexBufferMemory );
}

std::uint32_t Buffer::getMemoryTypeIndex( const std::uint32_t bufferMemoryTypeBits,
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

vk::Buffer Buffer::getHandler() const noexcept {
    return m_vertexBuffer;
}

std::uint32_t Buffer::getVerticesCount() const noexcept {
    return static_cast< std::uint32_t >( std::size( m_vertices ) );
}

} // namespace ve
