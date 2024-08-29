#pragma once

#include "Swapchain.hpp"
#include "Pipeline.hpp"

namespace ve {

class CommandBuffer {
public:
    CommandBuffer( const vk::CommandBuffer& commandBufferHandler, const ve::Swapchain& swapchain,
                   const ve::Pipeline& pipeline );

    CommandBuffer( const CommandBuffer& other ) = delete;
    CommandBuffer( CommandBuffer&& other )      = delete;

    CommandBuffer& operator=( const CommandBuffer& other ) = delete;
    CommandBuffer& operator=( CommandBuffer&& other )      = delete;

    ~CommandBuffer() = default;

    void record( const std::uint32_t imageIndex ) const;
    void reset();
    vk::CommandBuffer getHandler() const noexcept;

private:
    vk::CommandBuffer m_commandBuffer;
    const ve::Swapchain& m_swapchain;
    const ve::Pipeline& m_pipeline;
};

} // namespace ve
