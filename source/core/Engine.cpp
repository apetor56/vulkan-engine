#include "Engine.hpp"
#include "Config.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <chrono>

namespace ve {

std::vector< DescriptorAllocator::PoolSizeRatio > g_poolSizes = { { vk::DescriptorType::eStorageImage, 1 },
                                                                  { vk::DescriptorType::eUniformBuffer, 1 } };

Engine::Engine()
    : m_window{ WindowInfo{ cfg::window::width, cfg::window::height, "example" }, m_vulkanInstance },
      m_physicalDevice{ m_vulkanInstance, m_window },
      m_logicalDevice{ m_physicalDevice },
      m_memoryAllocator{ m_vulkanInstance, m_physicalDevice, m_logicalDevice },
      m_swapchain{ m_logicalDevice, m_window },
      m_vertexShader{ cfg::directory::shaderBinaries / "Simple.vert.spv", m_logicalDevice },
      m_fragmentShader{ cfg::directory::shaderBinaries / "Simple.frag.spv", m_logicalDevice },
      m_pipelineBuilder{ m_logicalDevice },
      m_graphicsCommandPool{ m_logicalDevice },
      m_immediateSubmitFence{ m_logicalDevice },
      m_immediateBuffer{ m_graphicsCommandPool.createCommandBuffers() },
      m_transferCommandPool{ m_logicalDevice },
      m_transferCommandBuffer{ m_transferCommandPool.createCommandBuffers() },
      m_descriptorSetLayout{ m_logicalDevice },
      m_loader{ *this, m_memoryAllocator },
      m_descriptorWriter{ m_logicalDevice },
      m_metalRoughMaterial{ m_logicalDevice },
      m_globalDescriptorAllocator{ m_logicalDevice, 10U, g_poolSizes } {
    init();
}

Engine::~Engine() {
    m_logicalDevice.get().destroySampler( m_textureSampler );
}

void Engine::init() {
    createDepthBuffer();
    createRenderPass();
    createFramebuffers();
    preparePipelines();
    createFrameResoures();
    prepareTexture();
    createTextureSampler();
    configureDescriptorSets();
    initDefaultData();
    loadMeshes();
}

void Engine::run() {
    while ( m_window.shouldClose() == GLFW_FALSE ) {
        glfwPollEvents();

        updateScene();

        const auto& currentFrame{ m_currentFrameIt->value() };
        [[maybe_unused]] const auto waitForFencesResult{
            m_logicalDevice.get().waitForFences( currentFrame.renderFence.get(), g_waitForAllFences, g_timeoutOff ) };

        const auto imageIndex{ acquireNextImage() };
        if ( !imageIndex.has_value() )
            return;

        m_logicalDevice.get().resetFences( currentFrame.renderFence.get() );

        draw( imageIndex.value() );
        present( imageIndex.value() );

        m_currentFrameIt++;
        if ( m_currentFrameIt == std::end( m_frameResources ) )
            m_currentFrameIt = std::begin( m_frameResources );
    }
    m_logicalDevice.get().waitIdle();
}

std::optional< uint32_t > Engine::acquireNextImage() {
    try {
        const auto& currentFrame{ m_currentFrameIt->value() };
        auto [ result, imageIndex ]{ m_logicalDevice.get().acquireNextImageKHR(
            m_swapchain.get(), g_timeoutOff, currentFrame.swapchainSemaphore.get() ) };

        if ( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR )
            throw std::runtime_error( "failed to acquire swapchain image" );

        return imageIndex;
    }
    catch ( const vk::OutOfDateKHRError& ) {
        handleWindowResising();
        return std::nullopt;
    }
}

void Engine::draw( const uint32_t imageIndex ) {
    auto& currentFrame{ m_currentFrameIt->value() };

    updateUniformBuffer();

    const auto& commandBuffer{ currentFrame.graphicsCommandBuffer };
    const auto commandBufferVk{ commandBuffer.get() };
    const auto renderFinishedSemaphore{ currentFrame.renderSemaphore.get() };
    const auto swapchainSemaphore{ currentFrame.swapchainSemaphore.get() };

    commandBuffer.reset();
    commandBuffer.begin();

    if ( !m_renderPass.has_value() )
        throw std::runtime_error( "renderpass not initialized" );
    if ( !m_pipeline.has_value() )
        throw std::runtime_error( "pipeline not initialized" );

    commandBuffer.beginRenderPass( m_renderPass->get(), m_framebuffers.at( imageIndex )->get(),
                                   m_swapchain.getExtent() );
    commandBuffer.setViewport( m_swapchain.getViewport() );
    commandBuffer.setScissor( m_swapchain.getScissor() );

    commandBuffer.bindPipeline( m_pipeline->get() );

    auto currentDescriptorSet{ currentFrame.descriptorSet };

    std::ranges::for_each(
        m_mainRenderContext.opaqueSurfaces, [ &commandBuffer, currentDescriptorSet, this ]( const auto& renderObject ) {
            commandBuffer.bindPipeline( renderObject.material.pipeline.get() );
            commandBuffer.bindDescriptorSet( renderObject.material.pipeline.getLayout(), currentDescriptorSet, 0U );
            commandBuffer.bindDescriptorSet( renderObject.material.pipeline.getLayout(),
                                             renderObject.material.descriptorSet, 1U );
            commandBuffer.bindIndexBuffer( renderObject.indexBuffer );

            const ve::PushConstants pushConstants{ .worldMatrix{ renderObject.transform },
                                                   .vertexBufferAddress{ renderObject.vertexBufferAddress } };
            commandBuffer.pushConstants( renderObject.material.pipeline.getLayout(), vk::ShaderStageFlagBits::eVertex,
                                         pushConstants );
            commandBuffer.drawIndices( renderObject.firstIndex, renderObject.indexCount );
        } );

    commandBuffer.endRenderPass();
    commandBuffer.end();

    static constexpr vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::SubmitInfo submitInfo{};
    submitInfo.sType                = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount   = 1U;
    submitInfo.pWaitSemaphores      = &swapchainSemaphore;
    submitInfo.pWaitDstStageMask    = &waitStage;
    submitInfo.commandBufferCount   = 1U;
    submitInfo.pCommandBuffers      = &commandBufferVk;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores    = &renderFinishedSemaphore;

    const auto graphicsQueue{ m_logicalDevice.getQueue( ve::QueueType::eGraphics ) };
    graphicsQueue.submit( submitInfo, currentFrame.renderFence.get() );
}

void Engine::present( const uint32_t imageIndex ) {
    const auto swapchainVk{ m_swapchain.get() };
    const auto& currentFrame{ m_currentFrameIt->value() };
    const auto renderSemaphore{ currentFrame.renderSemaphore.get() };

    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pWaitSemaphores    = &renderSemaphore;
    presentInfo.swapchainCount     = 1U;
    presentInfo.pSwapchains        = &swapchainVk;
    presentInfo.pImageIndices      = &imageIndex;

    const auto presentationQueue{ m_logicalDevice.getQueue( ve::QueueType::ePresentation ) };
    try {
        const auto presentResult{ presentationQueue.presentKHR( presentInfo ) };
        if ( presentResult == vk::Result::eSuboptimalKHR || m_window.isResized() )
            handleWindowResising();
    }
    catch ( const vk::OutOfDateKHRError& ) {
        handleWindowResising();
    }
}

void Engine::createDepthBuffer() {
    m_depthBuffer.emplace( m_memoryAllocator, m_logicalDevice, m_swapchain.getExtent(), vk::Format::eD32Sfloat,
                           vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth );
}

void Engine::createRenderPass() {
    m_renderPass.emplace( m_logicalDevice, m_swapchain.getFormat(), m_depthBuffer->getFormat() );
}

void Engine::createFramebuffers() {
    const auto& swapchainImageViews{ m_swapchain.getImageViews() };
    m_framebuffers.clear();
    m_framebuffers.reserve( std::size( swapchainImageViews ) );
    std::ranges::for_each( swapchainImageViews, [ this ]( const auto swapchainImageView ) {
        const std::array< vk::ImageView, 2U > attachments{ swapchainImageView, m_depthBuffer->getImageView() };
        m_framebuffers.emplace_back( std::in_place, m_renderPass.value(), attachments, m_swapchain.getExtent() );
    } );
}

void Engine::preparePipelines() {
    m_descriptorSetLayout.addBinding( 0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex );
    m_descriptorSetLayout.create();
    const auto layoutVk{ m_descriptorSetLayout.get() };

    static constexpr vk::PushConstantRange bufferRange{ ve::PushConstants::defaultRange() };
    auto pipelineLayoutInfo{ ve::PipelineLayout::defaultInfo() };
    pipelineLayoutInfo.pPushConstantRanges    = &bufferRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1U;
    pipelineLayoutInfo.pSetLayouts            = &layoutVk;
    pipelineLayoutInfo.setLayoutCount         = 1U;

    m_pipelineLayout.emplace( m_logicalDevice, pipelineLayoutInfo );

    m_pipelineBuilder.setShaders( m_vertexShader, m_fragmentShader );
    m_pipelineBuilder.setLayout( m_pipelineLayout.value() );
    m_pipeline.emplace( m_pipelineBuilder, m_renderPass.value() );

    m_metalRoughMaterial.buildPipelines( m_descriptorSetLayout, m_renderPass.value() );
}

void Engine::createFrameResoures() {
    const auto graphicsCommandBuffers{ m_graphicsCommandPool.createCommandBuffers< g_maxFramesInFlight >() };

    for ( uint32_t frameID{ 0U }; frameID < g_maxFramesInFlight; frameID++ )
        m_frameResources.at( frameID ).emplace( sizeof( SceneData ), m_logicalDevice, m_memoryAllocator,
                                                graphicsCommandBuffers.at( frameID ), m_descriptorSetLayout );
    m_currentFrameIt = std::begin( m_frameResources );
}

void Engine::updateUniformBuffer() {
    const auto& currentFrame{ m_currentFrameIt->value() };
    memcpy( currentFrame.uniformBuffer.getMappedMemory(), &m_sceneData, sizeof( m_sceneData ) );
}

void Engine::configureDescriptorSets() {
    std::ranges::for_each( m_frameResources, [ this ]( auto& frameData ) {
        static constexpr uint32_t uniformBufferBinding{ 0U };
        m_descriptorWriter.writeBuffer( uniformBufferBinding, frameData.value().uniformBuffer.get(),
                                        sizeof( SceneData ), 0U, vk::DescriptorType::eUniformBuffer );

        m_descriptorWriter.updateSet( frameData.value().descriptorSet );
    } );

    std::vector< DescriptorAllocator::PoolSizeRatio > sizes = { { vk::DescriptorType::eStorageImage, 1 },
                                                                { vk::DescriptorType::eUniformBuffer, 1 } };
}

MeshBuffers Engine::uploadMeshBuffers( std::span< Vertex > vertices, std::span< uint32_t > indices ) const {
    const auto logicalDeviceVk{ m_logicalDevice.get() };
    const auto commandBufferVk{ m_transferCommandBuffer.get() };

    MeshBuffers newMeshBuffers;
    newMeshBuffers.vertexBuffer.emplace( m_memoryAllocator, std::size( vertices ) * sizeof( Vertex ) );
    newMeshBuffers.indexBuffer.emplace( m_memoryAllocator, std::size( indices ) * sizeof( uint32_t ) );

    if ( newMeshBuffers.vertexBuffer.has_value() ) {
        vk::BufferDeviceAddressInfo addressInfo{};
        addressInfo.sType                  = vk::StructureType::eBufferDeviceAddressInfo;
        addressInfo.buffer                 = newMeshBuffers.vertexBuffer->get();
        newMeshBuffers.vertexBufferAddress = logicalDeviceVk.getBufferAddress( addressInfo );
    } else {
        throw std::runtime_error( "failed to obtain buffer address" );
    }

    const vk::DeviceSize vertexBufferSize{ std::size( vertices ) * sizeof( Vertex ) };
    const vk::DeviceSize indexBufferSize{ std::size( indices ) * sizeof( uint32_t ) };

    StagingBuffer stagingBuffer{ m_memoryAllocator, vertexBufferSize + indexBufferSize };
    void *mappedMemory{ stagingBuffer.getMappedMemory() };
    memcpy( mappedMemory, std::data( vertices ), vertexBufferSize );
    memcpy( static_cast< char * >( mappedMemory ) + vertexBufferSize, std::data( indices ), indexBufferSize );

    logicalDeviceVk.resetFences( m_immediateSubmitFence.get() );
    m_transferCommandBuffer.reset();

    m_transferCommandBuffer.begin();

    constexpr vk::DeviceSize vertexSrcOffset{ 0U };
    constexpr vk::DeviceSize vertexDstOffset{ 0U };
    m_transferCommandBuffer.copyBuffer( vertexSrcOffset, vertexDstOffset, vertexBufferSize, stagingBuffer.get(),
                                        newMeshBuffers.vertexBuffer->get() );

    const vk::DeviceSize indexSrcOffset{ vertexBufferSize };
    constexpr vk::DeviceSize indexDstOffset{ 0U };
    m_transferCommandBuffer.copyBuffer( indexSrcOffset, indexDstOffset, indexBufferSize, stagingBuffer.get(),
                                        newMeshBuffers.indexBuffer->get() );

    m_transferCommandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.sType              = vk::StructureType::eSubmitInfo;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers    = &commandBufferVk;

    const auto transferQueue{ m_logicalDevice.getQueue( ve::QueueType::eTransfer ) };
    transferQueue.submit( submitInfo, m_immediateSubmitFence.get() );
    [[maybe_unused]] const auto result{
        logicalDeviceVk.waitForFences( m_immediateSubmitFence.get(), g_waitForAllFences, g_timeoutOff ) };

    return newMeshBuffers;
}

void Engine::prepareTexture() {
    int width;
    int height;
    int channels;
    const auto fullTexturePath{ cfg::directory::assets / "viking_room.png" };

    stbi_uc *pixels{ stbi_load( fullTexturePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha ) };

    if ( !pixels )
        throw std::runtime_error( "failed to load texture image" );

    const vk::DeviceSize bufferSize{ static_cast< vk::DeviceSize >( width ) * height * 4 };
    ve::StagingBuffer stagingBuffer{ m_memoryAllocator, bufferSize };
    memcpy( stagingBuffer.getMappedMemory(), pixels, bufferSize );

    const vk::Extent2D imageExtent{ static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) };
    m_textureImage.emplace( m_memoryAllocator, m_logicalDevice, imageExtent, vk::Format::eR8G8B8A8Srgb,
                            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                            vk::ImageAspectFlagBits::eColor );

    immediateSubmit( [ &stagingBuffer, this ]( ve::GraphicsCommandBuffer cmd ) {
        cmd.transitionImageBuffer( m_textureImage->get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal );
        cmd.copyBufferToImage( stagingBuffer.get(), m_textureImage->get(), m_textureImage->getExtent() );
        cmd.transitionImageBuffer( m_textureImage->get(), vk::Format::eR8G8B8A8Srgb,
                                   vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal );
    } );
}

void Engine::createTextureSampler() {
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter               = vk::Filter::eLinear;
    samplerInfo.minFilter               = vk::Filter::eLinear;
    samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable        = vk::True;
    samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable           = vk::False;
    samplerInfo.compareOp               = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias              = 0.0F;
    samplerInfo.minLod                  = 0.0F;
    samplerInfo.maxLod                  = 0.0F;

    const auto physicalDeviceProperties{ m_physicalDevice.get().getProperties() };
    samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

    m_textureSampler = m_logicalDevice.get().createSampler( samplerInfo );
}

void Engine::loadMeshes() {
    m_modelMeshes = m_loader.loadMeshes( cfg::directory::assets / "viking_room.obj" );
    std::ranges::for_each( m_modelMeshes, [ this ]( auto& meshAsset ) {
        std::shared_ptr< MeshNode > meshNode{ std::make_shared< MeshNode >( meshAsset ) };
        meshAsset.surface.material = std::make_shared< GltfMaterial >( m_defaultMaterial.value() );
        m_nodes.emplace( meshAsset.name, std::move( meshNode ) );
    } );
}

void Engine::handleWindowResising() {
    m_swapchain.recreate();
    createRenderPass();
    createDepthBuffer();
    createFramebuffers();
}

void Engine::immediateSubmit( const std::function< void( ve::GraphicsCommandBuffer command ) >& function ) {
    const auto logicalDeviceVk{ m_logicalDevice.get() };
    logicalDeviceVk.resetFences( m_immediateSubmitFence.get() );
    m_immediateBuffer.reset();

    ve::GraphicsCommandBuffer command{ m_immediateBuffer };
    command.begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
    function( command );
    command.end();

    const auto commandHanlder{ command.get() };
    vk::SubmitInfo submitInfo{};
    submitInfo.sType              = vk::StructureType::eSubmitInfo;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers    = &commandHanlder;

    m_logicalDevice.getQueue( ve::QueueType::eGraphics ).submit( submitInfo, m_immediateSubmitFence.get() );
    [[maybe_unused]] const auto waitForFencesResult{
        logicalDeviceVk.waitForFences( m_immediateSubmitFence.get(), g_waitForAllFences, g_timeoutOff ) };
}

void Engine::updateScene() {
    m_mainRenderContext.opaqueSurfaces.clear();

    using namespace std::chrono;
    static auto start{ high_resolution_clock::now() };
    auto now{ high_resolution_clock::now() };
    milliseconds elapsed{ duration_cast< milliseconds >( now - start ) };

    constexpr glm::vec3 zAxis{ 0.0F, 0.0F, 1.0F };
    m_sceneData.model = glm::scale( glm::mat4{ 1.0F }, glm::vec3{ 1.0F, 1.0F, 1.0F } ) *
                        glm::rotate( glm::mat4( 1.0F ), elapsed.count() * glm::radians( 90.0F ) / 1000, zAxis );

    constexpr glm::vec3 cameraPos{ 2.0F, 4.0F, 2.0F };
    constexpr glm::vec3 centerPos{};
    constexpr glm::vec3 up{ 0.0F, 0.0F, 1.0F };
    m_sceneData.view = glm::lookAt( cameraPos, centerPos, up );

    static const auto& extent{ m_swapchain.getExtent() };
    constexpr float angle{ 45.0F };
    constexpr float nearPlane{ 0.1F };
    constexpr float farPlane{ 500.0F };
    m_sceneData.projection = glm::perspective(
        glm::radians( angle ), static_cast< float >( extent.width ) / extent.height, nearPlane, farPlane );
    m_sceneData.projection[ 1 ][ 1 ] *= -1;

    std::ranges::for_each( m_nodes | std::views::values, [ this ]( auto& meshNode ) {
        meshNode->render( glm::mat4{ 1.0F }, m_mainRenderContext );
        glm::mat4 translation{ glm::translate( glm::mat4{ 1.0F }, glm::vec3{ 2, 0, 0 } ) };
        glm::mat4 scale{ glm::scale( glm::mat4{ 1.0F }, glm::vec3{ 0.5F } ) };
        meshNode->render( translation * scale, m_mainRenderContext );
    } );
}

void Engine::initDefaultData() {
    m_materialConstantsUniformBuffer.emplace( m_memoryAllocator, sizeof( GltfMetalicRoughness::Constants ) );
    GltfMetalicRoughness::Constants *materialConstants{
        static_cast< GltfMetalicRoughness::Constants * >( m_materialConstantsUniformBuffer->getMappedMemory() ) };
    materialConstants->colorFactors            = glm::vec4{ 1.0F, 1.0F, 1.0F, 1.0F };
    materialConstants->metalicRoughnessFactors = glm::vec4{ 1.0F, 0.0F, 0.0F, 0.0F };

    static constexpr uint32_t offset{ 0U };
    m_defaultResources.emplace( m_textureImage->getImageView(), m_textureImage->getImageView(), m_textureSampler,
                                m_textureSampler, m_materialConstantsUniformBuffer->get(), offset );

    m_defaultMaterial.emplace( m_metalRoughMaterial.writeMaterial(
        Material::Type::eMainColor, m_defaultResources.value(), m_globalDescriptorAllocator ) );
}

} // namespace ve
