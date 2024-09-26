#pragma once

#include "Vertex.hpp"
#include "MemoryAllocator.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

struct UniformBufferData {
    glm::mat4 model{ 1.0F };
    glm::mat4 view{ 1.0F };
    glm::mat4 projection{ 1.0F };

    constexpr auto size() const { return 1U; }
    auto data() const { return this; }
};

namespace ve {

template < typename T >
concept BufferData = requires( T data ) {
    data.size();
    data.data();
};

template < VkBufferUsageFlags bufferUsage, BufferData T >
class Buffer {
public:
    Buffer( const ve::MemoryAllocator& memoryAllocator, T data )
        : m_memoryAllocator{ memoryAllocator }, m_dataCount{ static_cast< std::uint32_t >( std::size( data ) ) } {
        const auto dataSizeInBytes{ sizeof( T ) * std::size( data ) };

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.usage = bufferUsage;
        bufferCreateInfo.size  = dataSizeInBytes;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags =
            VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VmaAllocationInfo allocationInfo{};
        vmaCreateBuffer( m_memoryAllocator, &bufferCreateInfo, &allocationCreateInfo, &m_buffer, &m_allocation,
                         &allocationInfo );

        memcpy( allocationInfo.pMappedData, std::data( data ), dataSizeInBytes );
    }

    ~Buffer() { vmaDestroyBuffer( m_memoryAllocator, m_buffer, m_allocation ); }

    vk::Buffer getHandler() const noexcept { return m_buffer; }
    std::uint32_t getCount() const noexcept { return m_dataCount; }
    void *getMappedMemory() const noexcept {
        VmaAllocationInfo allocationInfo{};
        vmaGetAllocationInfo( m_memoryAllocator, m_allocation, &allocationInfo );
        return allocationInfo.pMappedData;
    }

private:
    const ve::MemoryAllocator& m_memoryAllocator;
    VmaAllocation m_allocation;
    VkBuffer m_buffer;
    std::uint32_t m_dataCount;
};

using VertexBuffer  = Buffer< VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, std::vector< Vertex > >;
using IndexBuffer   = Buffer< VK_BUFFER_USAGE_INDEX_BUFFER_BIT, std::vector< std::uint32_t > >;
using UniformBuffer = Buffer< VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, UniformBufferData >;

} // namespace ve
