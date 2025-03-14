#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "MemoryAllocator.hpp"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "ShaderModule.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Frame.hpp"
#include "Constants.hpp"
#include "command/CommandPool.hpp"
#include "command/GraphicsCommandBuffer.hpp"
#include "descriptor/DescriptorSetLayout.hpp"
#include "descriptor/DescriptorWriter.hpp"
#include "Loader.hpp"
#include "Material.hpp"

#include <functional>

namespace ve {

class Engine {
public:
    Engine();
    ~Engine();

    void init();
    void run();

    MeshBuffers uploadMeshBuffers( std::span< Vertex > vertices, std::span< uint32_t > indices ) const;

private:
    using FrameResources = std::array< std::optional< ve::FrameData >, g_maxFramesInFlight >;
    using Framebuffers   = std::vector< std::optional< ve::Framebuffer > >;

    ve::VulkanInstance m_vulkanInstance{};
    ve::Window m_window;
    ve::PhysicalDevice m_physicalDevice;
    ve::LogicalDevice m_logicalDevice;
    ve::MemoryAllocator m_memoryAllocator;
    ve::Swapchain m_swapchain;
    Framebuffers m_framebuffers;
    std::optional< ve::Image > m_depthBuffer{};
    std::optional< ve::RenderPass > m_renderPass{};
    ve::ShaderModule m_vertexShader;
    ve::ShaderModule m_fragmentShader;
    std::optional< ve::PipelineLayout > m_pipelineLayout{};
    ve::PipelineBuilder m_pipelineBuilder;
    std::optional< ve::Pipeline > m_pipeline{};
    ve::CommandPool< ve::GraphicsCommandBuffer > m_graphicsCommandPool;
    ve::Fence m_immediateSubmitFence;
    ve::GraphicsCommandBuffer m_immediateBuffer;
    ve::CommandPool< ve::TransferCommandBuffer > m_transferCommandPool;
    ve::TransferCommandBuffer m_transferCommandBuffer;
    ve::MeshBuffers m_meshBuffers{};
    ve::DescriptorSetLayout m_descriptorSetLayout;
    FrameResources m_frameResources;
    FrameResources::iterator m_currentFrameIt{ nullptr };
    std::optional< ve::Image > m_textureImage{};
    vk::Sampler m_sampler;
    ve::Loader m_loader;
    std::vector< ve::MeshAsset > m_modelMeshes;
    ve::DescriptorWriter m_descriptorWriter;
    ve::Material m_material;
    ve::GltfMetalicRoughness m_metalRoughMaterial;

    void createDepthBuffer();
    void createRenderPass();
    void createFramebuffers();
    void preparePipeline();
    void createFrameResoures();
    void updateUniformBuffer();
    void configureDescriptorSets();
    void prepareTexture();
    void createTextureSampler();
    void loadMeshes();

    std::optional< uint32_t > acquireNextImage();
    void draw( const uint32_t imageIndex );
    void present( const uint32_t imageIndex );

    void handleWindowResising();
    void immediateSubmit( const std::function< void( GraphicsCommandBuffer command ) >& function );
};

} // namespace ve
