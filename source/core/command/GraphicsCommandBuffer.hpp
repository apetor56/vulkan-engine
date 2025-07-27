#pragma once

#include "BaseCommandBuffer.hpp"

namespace ve {

class LogicalDevice;
struct PushConstants;

class GraphicsCommandBuffer : public BaseCommandBuffer {
public:
    using BaseCommandBuffer::BaseCommandBuffer;
    using BaseCommandBuffer::operator=;

    static uint32_t getQueueFamilyID( const ve::LogicalDevice& logicalDevice );

    void bindPipeline( const vk::Pipeline pipeline ) const noexcept;
    void setViewport( const vk::Viewport viewport ) const noexcept;
    void setScissor( const vk::Rect2D scissor ) const noexcept;
    void bindVertexBuffer( const vk::Buffer vertexBuffer ) const;
    void bindIndexBuffer( const vk::Buffer indexBuffer ) const;
    void bindDescriptorSet( const vk::PipelineLayout pipelineLayout, const vk::DescriptorSet descriptorSet,
                            const uint32_t firstSet = 0U ) const noexcept;
    void drawIndices( const uint32_t firstIndex, const uint32_t indicesCount ) const noexcept;
    void transitionImageLayout( const vk::Image image, const vk::Format format, const vk::ImageLayout oldLayout,
                                const vk::ImageLayout newLayout, const uint32_t mipLevel = 1U ) const;
    void copyBufferToImage( const vk::Buffer buffer, const vk::Image image, const vk::Extent2D extent );
    void pushConstants( const vk::PipelineLayout layout, const vk::ShaderStageFlags shaderStages,
                        const ve::PushConstants& pushConstants, const uint32_t offset = 0U ) const noexcept;
    void beginRendering( const vk::Extent2D extent, const vk::ImageView sampledImageView,
                         const vk::ImageView resolvedImageView, const vk::ImageView depthView ) const;
    void endRendering() const;
};

} // namespace ve
