#pragma once

#include "BaseCommandBuffer.hpp"

namespace ve {

class GraphicsCommandBuffer : public BaseCommandBuffer {
public:
    using BaseCommandBuffer::BaseCommandBuffer;
    using BaseCommandBuffer::operator=;

    void beginRenderPass( const vk::RenderPass renderPass, const vk::Framebuffer framebuffer,
                          const vk::Extent2D renderArea ) const noexcept;
    void bindPipeline( const vk::Pipeline pipeline ) const noexcept;
    void setViewport( const vk::Viewport viewport ) const noexcept;
    void setScissor( const vk::Rect2D scissor ) const noexcept;
    void bindVertexBuffer( const vk::Buffer vertexBuffer ) const;
    void bindIndexBuffer( const vk::Buffer indexBuffer ) const;
    void draw( const std::uint32_t verticesCount ) const noexcept;
    void drawIndices( const std::uint32_t indicesCount ) const noexcept;
    void endRenderPass() const noexcept;
};

} // namespace ve
