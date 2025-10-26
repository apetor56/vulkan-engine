#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "MemoryAllocator.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Frame.hpp"
#include "Constants.hpp"
#include "Loader.hpp"
#include "Material.hpp"
#include "Node.hpp"
#include "Camera.hpp"
#include "ShaderModule.hpp"

#include "command/CommandPool.hpp"
#include "command/GraphicsCommandBuffer.hpp"

#include "descriptor/DescriptorSetLayout.hpp"
#include "descriptor/DescriptorWriter.hpp"

#include <functional>

namespace ve {

class Engine {
public:
    Engine();

    void init();
    void run();

    MeshBuffers uploadMeshBuffers( std::span< Vertex > vertices, std::span< uint32_t > indices ) const;
    ve::Image createImage( void *data, const vk::Extent2D size, const vk::Format format,
                           const vk::ImageUsageFlags usage, const uint32_t mipLevels = 1U );

    const ve::LogicalDevice& getLogicalDevice() const noexcept { return m_logicalDevice; }
    const ve::Image& getDefaultImage() const noexcept { return m_defaultWhiteImage.value(); }
    const ve::Sampler& getDefaultSampler() const noexcept { return m_defaultTextureSampler.value(); }
    const ve::Material& getDefaultMaterial() const noexcept { return m_defaultMaterial.value(); }
    ve::gltf::MetalicRoughness& getMaterialBuiler() noexcept { return m_metalRough; }

private:
    using FrameResources = std::array< std::optional< ve::FrameData >, g_maxFramesInFlight >;
    using Scene          = std::unordered_map< std::string_view, std::shared_ptr< ve::gltf::Scene > >;

    struct SceneData {
        glm::mat4 model{ 1.0F };
        glm::mat4 view{ 1.0F };
        glm::mat4 projection{ 1.0F };
        glm::vec4 lightColor{};
        glm::vec3 lightPosition{};
        int allignement01;
        glm::vec3 cameraPosition;
        int allignment02;
    };

    ve::VulkanInstance m_vulkanInstance{};
    ve::Window m_window;
    ve::PhysicalDevice m_physicalDevice;
    ve::LogicalDevice m_logicalDevice;
    ve::MemoryAllocator m_memoryAllocator;
    ve::Swapchain m_swapchain;
    std::optional< ve::Image > m_colorImage{};
    std::optional< ve::Image > m_depthBuffer{};
    std::optional< ve::PipelineLayout > m_pipelineLayout{};
    ve::PipelineBuilder m_pipelineBuilder;
    ve::CommandPool< ve::GraphicsCommandBuffer > m_graphicsCommandPool;
    ve::Fence m_immediateSubmitFence;
    ve::GraphicsCommandBuffer m_immediateBuffer;
    ve::CommandPool< ve::TransferCommandBuffer > m_transferCommandPool;
    ve::TransferCommandBuffer m_transferCommandBuffer;
    ve::MeshBuffers m_meshBuffers{};
    ve::DescriptorSetLayout m_descriptorSetLayout;
    FrameResources m_frameResources;
    FrameResources::iterator m_currentFrameIt{ nullptr };
    std::optional< ve::Image > m_defaultWhiteImage{};
    std::optional< ve::Sampler > m_defaultTextureSampler;
    ve::gltf::Loader m_loader;
    std::vector< ve::MeshAsset > m_modelMeshes;
    ve::DescriptorWriter m_descriptorWriter;
    std::optional< ve::Material > m_material;
    ve::gltf::MetalicRoughness m_metalRough;
    ve::DescriptorAllocator m_globalDescriptorAllocator;
    std::optional< ve::Material > m_defaultMaterial;
    ve::gltf::MetalicRoughness::Resources m_defaultResources;
    std::optional< ve::UniformBuffer > m_constantsBuffer;
    ve::RenderContext m_mainRenderContext;
    SceneData m_sceneData{};
    Scene m_scene;
    std::shared_ptr< ve::Camera > m_camera{};

    std::optional< ve::Pipeline > m_skyboxPipeline;
    std::optional< ve::PipelineLayout > m_skyboxPipelineLayout;
    ve::DescriptorSetLayout m_skyboxDescriptorSetLayout;
    vk::DescriptorSet m_skyboxDescriptorSet;
    ve::ShaderModule m_skyboxVertexShader;
    ve::ShaderModule m_skyboxFragmentShader;
    std::optional< ve::Image > m_skyboxImage;
    std::optional< ve::Sampler > m_skyboxSampler;

    std::optional< ve::Pipeline > m_hdrPipeline;
    std::optional< ve::PipelineLayout > m_hdrPipelineLayout;
    ve::DescriptorSetLayout m_hdrDescriptorSetLayout;
    vk::DescriptorSet m_hdrDescriptorSet;
    ve::ShaderModule m_hdrVertexShader;
    ve::ShaderModule m_hdrFragmentShader;
    std::optional< ve::Image > m_hdrImage;
    std::optional< ve::Sampler > m_hdrSampler;
    std::optional< ve::Image > m_hdrSkybox;
    std::optional< ve::UniformBufferGPU > m_hdrUniformBuf;

    void createColorResources();
    void createDepthBuffer();
    void preparePipelines();
    void createFrameResoures();
    void updateUniformBuffer();
    void configureDescriptorSets();
    void prepareDefaultTexture();
    void createDefaultTextureSampler();
    void loadMeshes();
    void initDefaultData();
    void generateMipmaps( const ve::Image& image, const int32_t texWidth, const int32_t texHeight,
                          const uint32_t mipLevels );
    void prepareSkyboxTexture();
    void createSkybox();
    void loadHDRI();

    void updateScene( float deltaTime );
    std::optional< uint32_t > acquireNextImage();
    void draw( const uint32_t imageIndex );
    void present( const uint32_t imageIndex );

    void drawScene( const ve::GraphicsCommandBuffer currentCommandBuffer, const vk::DescriptorSet currentGlobalSet );
    void drawSkybox( const ve::GraphicsCommandBuffer currentCommandBuffer, const vk::DescriptorSet currentGlobalSet );
    void drawHDRI( const ve::GraphicsCommandBuffer currentCommandBuffer, const vk::DescriptorSet currentGlobalSet );
    void renderHDRSkybox();

    void handleWindowResising();
    void immediateSubmit( const std::function< void( GraphicsCommandBuffer command ) >& function );
};

} // namespace ve
