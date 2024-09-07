#pragma once

#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"

namespace ve {

class CommandBuffer {
public:
    CommandBuffer( const vk::CommandBuffer& commandBufferHandler, const ve::Swapchain& swapchain,
                   const ve::Pipeline& pipeline );

    CommandBuffer( const CommandBuffer& other ) = delete;
    CommandBuffer( CommandBuffer&& other )      = default;

    CommandBuffer& operator=( const CommandBuffer& other ) = delete;
    CommandBuffer& operator=( CommandBuffer&& other )      = delete;

    ~CommandBuffer() = default;

    void record( const std::uint32_t imageIndex ) const;

    void setData( std::shared_ptr< ve::VertexBuffer > dataBuffer ) noexcept;
    void reset();
    vk::CommandBuffer getHandler() const noexcept;

private:
    vk::CommandBuffer m_commandBuffer;
    const ve::Swapchain& m_swapchain;
    const ve::Pipeline& m_pipeline;
    std::shared_ptr< ve::VertexBuffer > m_vertexBuffer{};
};

} // namespace ve
