#pragma once

#include "Vertex.hpp"
#include "MemoryAllocator.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <span>

struct UniformBufferData {
    glm::mat4 model{ 1.0F };
    glm::mat4 view{ 1.0F };
    glm::mat4 projection{ 1.0F };
};

namespace ve {

template < VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags allocationFlags = VmaAllocationCreateFlags{} >
class Buffer {
public:
    Buffer( const ve::MemoryAllocator& memoryAllocator, VkDeviceSize size )
        : m_memoryAllocator{ memoryAllocator }, m_size{ size } {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.usage = bufferUsage;
        bufferCreateInfo.size  = size;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = allocationFlags;

        VmaAllocationInfo allocationInfo{};
        vmaCreateBuffer( m_memoryAllocator, &bufferCreateInfo, &allocationCreateInfo, &m_buffer, &m_allocation,
                         &allocationInfo );
    }

    ~Buffer() { vmaDestroyBuffer( m_memoryAllocator, m_buffer, m_allocation ); }

    Buffer( const Buffer& other ) = delete;
    Buffer( Buffer&& other ) : m_memoryAllocator{ other.m_memoryAllocator } {
        m_allocation = other.m_allocation;
        m_buffer     = other.m_buffer;
        m_size       = other.m_size;

        other.m_allocation = {};
        other.m_buffer     = {};
    }

    Buffer& operator=( const Buffer& other ) = delete;
    Buffer& operator=( Buffer&& other ) {
        m_allocation = other.m_allocation;
        m_buffer     = other.m_buffer;
        m_size       = other.m_size;

        other.m_buffer = {};

        return *this;
    }

    vk::Buffer getHandler() const noexcept { return m_buffer; }

    VkDeviceSize size() const noexcept { return m_size; }

    void *getMappedMemory() const noexcept {
        VmaAllocationInfo allocationInfo{};
        vmaGetAllocationInfo( m_memoryAllocator, m_allocation, &allocationInfo );
        return allocationInfo.pMappedData;
    }

private:
    const ve::MemoryAllocator& m_memoryAllocator;
    VmaAllocation m_allocation;
    VkBuffer m_buffer;
    VkDeviceSize m_size{};
};

using StagingBuffer = Buffer< VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT >;
using VertexBuffer  = Buffer< VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT >;
using IndexBuffer   = Buffer< VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT >;
using UniformBuffer = Buffer< VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                              VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT >;
} // namespace ve
