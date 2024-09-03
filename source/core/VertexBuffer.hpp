#pragma once

#include "Vertex.hpp"
#include "LogicalDevice.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

namespace ve {

class VertexBuffer {
public:
    VertexBuffer( const ve::LogicalDevice& logicalDevice );
    ~VertexBuffer();

    VertexBuffer( const VertexBuffer& other ) = delete;
    VertexBuffer( VertexBuffer&& other )      = delete;

    VertexBuffer& operator=( const VertexBuffer& other ) = delete;
    VertexBuffer& operator=( VertexBuffer&& other )      = delete;

    vk::Buffer getHandler() const noexcept;
    std::uint32_t getVerticesCount() const noexcept;

private:
    std::vector< Vertex > m_vertices{ { { 0.0F, -0.5F, 0.0F }, { 1.0F, 0.0F, 0.0F } },
                                      { { 0.5F, 0.5F, 0.0F }, { 0.0F, 1.0F, 0.0F } },
                                      { { -0.5F, 0.5F, 0.0F }, { 0.0F, 0.0F, 1.0F } } };
    vk::Buffer m_vertexBuffer{};
    vk::DeviceMemory m_vertexBufferMemory{};
    const ve::LogicalDevice& m_logicalDevice;
    inline static std::uint32_t s_bufferOffset{ 0U };

    void createVertexBuffer();
    void allocateMemory();
    void bindBufferMemory();
    void setData();
    std::uint32_t getMemoryTypeIndex( const std::uint32_t bufferMemoryTypeBits,
                                      vk::MemoryPropertyFlags wantedPropertiesFlag ) const;
};

} // namespace ve
