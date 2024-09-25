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

template < typename T >
concept BufferData = requires( T data ) {
    data.size();
    data.data();
};

namespace ve {

template < VkBufferUsageFlagBits bufferUsage, VmaAllocationCreateFlags allocationFlags, BufferData T >
class Buffer {
public:
    Buffer( const ve::MemoryAllocator& memoryAllocator, T data )
        : m_memoryAllocator{ memoryAllocator }, m_dataCount{ static_cast< std::uint32_t >( std::size( data ) ) } {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = bufferUsage;
        bufferInfo.size  = sizeof( T ) * std::size( data );

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = allocationFlags;

        VmaAllocationInfo allocationInfo{};
        vmaCreateBuffer( m_memoryAllocator, &bufferCreateInfo, &allocationCreateInfo, &m_buffer, &m_allocation,
                         &allocationInfo );
    }

    ~Buffer() { vmaDestroyBuffer( m_memoryAllocator, m_buffer, m_allocation ); }

    vk::Buffer getHandler() const noexcept { return m_buffer; }
    std::uint32_t getCount() const noexcept { return m_dataCount; }
    void *getMappedMemory() const noexcept { return m_cpuMemory; }

private:
    const ve::MemoryAllocator& m_memoryAllocator;
    VmaAllocation m_allocation;
    VkBuffer m_buffer;
    std::uint32_t m_dataCount;
};

using StagingBuffer = Buffer< VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                              std::vector< Vertex > >;
using VertexBuffer  = Buffer< VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VMA_MEMORY_USAGE_GPU_ONLY, std::vector< Vertex > >;
using IndexBuffer   = Buffer< vk::BufferUsageFlagBits::eIndexBuffer, std::vector< std::uint32_t > >;
using UniformBuffer = Buffer< vk::BufferUsageFlagBits::eUniformBuffer, UniformBufferData >;

} // namespace ve
