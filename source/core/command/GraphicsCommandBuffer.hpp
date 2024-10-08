#pragma once

#include "BaseCommandBuffer.hpp"

#include <cstdint>

namespace ve {

class LogicalDevice;

class GraphicsCommandBuffer : public BaseCommandBuffer {
public:
    using BaseCommandBuffer::BaseCommandBuffer;
    using BaseCommandBuffer::operator=;

    static std::uint32_t getQueueFamilyID( const ve::LogicalDevice& logicalDevice );

    void beginRenderPass( const vk::RenderPass renderPass, const vk::Framebuffer framebuffer,
                          const vk::Extent2D renderArea ) const noexcept;
    void bindPipeline( const vk::Pipeline pipeline ) const noexcept;
    void setViewport( const vk::Viewport viewport ) const noexcept;
    void setScissor( const vk::Rect2D scissor ) const noexcept;
    void bindVertexBuffer( const vk::Buffer vertexBuffer ) const;
    void bindIndexBuffer( const vk::Buffer indexBuffer ) const;
    void bindDescriptorSet( const vk::PipelineLayout pipelineLayout, const vk::DescriptorSet descriptorSet,
                            const std::uint32_t firstSet = 0U ) const noexcept;
    void draw( const std::uint32_t verticesCount ) const noexcept;
    void drawIndices( const std::uint32_t indicesCount ) const noexcept;
    void endRenderPass() const noexcept;
    void transitionImageBuffer( const vk::Image image, const vk::Format format, const vk::ImageLayout oldLayout,
                                const vk::ImageLayout newLayout );
    void copyBufferToImage( const vk::Buffer buffer, const vk::Image image, const vk::Extent2D extent );
};

} // namespace ve
