#include "Engine.hpp"
#include "Config.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/packing.hpp>

#include <spdlog/spdlog.h>

#include <limits>
#include <chrono>

namespace ve {

std::vector< DescriptorAllocator::PoolSizeRatio > g_poolSizes = { { vk::DescriptorType::eStorageImage, 1.0f },
                                                                  { vk::DescriptorType::eUniformBuffer, 1.0f },
                                                                  { vk::DescriptorType::eCombinedImageSampler, 1.0f } };

Engine::Engine()
    : m_window{ WindowInfo{ cfg::window::width, cfg::window::height, "example" }, m_vulkanInstance },
      m_physicalDevice{ m_vulkanInstance, m_window },
      m_logicalDevice{ m_physicalDevice },
      m_memoryAllocator{ m_vulkanInstance, m_physicalDevice, m_logicalDevice },
      m_swapchain{ m_logicalDevice, m_window },
      m_pipelineBuilder{ m_logicalDevice },
      m_graphicsCommandPool{ m_logicalDevice },
      m_immediateSubmitFence{ m_logicalDevice },
      m_immediateBuffer{ m_graphicsCommandPool.createCommandBuffers() },
      m_transferCommandPool{ m_logicalDevice },
      m_transferCommandBuffer{ m_transferCommandPool.createCommandBuffers() },
      m_descriptorSetLayout{ m_logicalDevice },
      m_loader{ *this, m_memoryAllocator },
      m_descriptorWriter{ m_logicalDevice },
      m_metalRough{ m_logicalDevice },
      m_globalDescriptorAllocator{ m_logicalDevice, 10U, g_poolSizes },
      m_camera{ std::make_shared< ve::Camera >() },
      m_skyboxDescriptorSetLayout{ m_logicalDevice },
      m_skyboxVertexShader{ cfg::directory::shaderBinaries / "Skybox.vert.spv", m_logicalDevice },
      m_skyboxFragmentShader{ cfg::directory::shaderBinaries / "Skybox.frag.spv", m_logicalDevice },
      m_hdrDescriptorSetLayout{ m_logicalDevice },
      m_hdrVertexShader{ cfg::directory::shaderBinaries / "HDR.vert.spv", m_logicalDevice },
      m_hdrFragmentShader{ cfg::directory::shaderBinaries / "HDR.frag.spv", m_logicalDevice } {
    init();
}

void Engine::init() {
    m_window.setCamera( m_camera );
    createColorResources();
    createDepthBuffer();
    preparePipelines();
    createFrameResoures();
    prepareDefaultTexture();
    createDefaultTextureSampler();
    configureDescriptorSets();
    initDefaultData();
    loadMeshes();
    loadHDRI();
    renderHDRSkybox();
    createSkybox();
}

void Engine::run() {
    using namespace std::chrono;
    high_resolution_clock::time_point frameStart{ high_resolution_clock::now() };
    high_resolution_clock::time_point now{};
    duration< float, std::milli > deltaTime{};

    while ( m_window.shouldClose() == GLFW_FALSE ) {
        now       = high_resolution_clock::now();
        deltaTime = now - frameStart;

        glfwPollEvents();
        updateScene( deltaTime.count() );

        const auto& currentFrame{ m_currentFrameIt->value() };
        [[maybe_unused]] const auto waitForFencesResult{
            m_logicalDevice.get().waitForFences( currentFrame.renderFence.get(), g_waitForAllFences, g_timeoutOff ) };
        m_logicalDevice.get().resetFences( currentFrame.renderFence.get() );

        const auto imageIndex{ acquireNextImage() };
        if ( !imageIndex.has_value() )
            return;

        draw( imageIndex.value() );
        present( imageIndex.value() );

        m_currentFrameIt++;
        if ( m_currentFrameIt == std::end( m_frameResources ) )
            m_currentFrameIt = std::begin( m_frameResources );

        frameStart = now;
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

    commandBuffer.transitionImageLayout( m_swapchain.getImage( imageIndex ), m_swapchain.getFormat(),
                                         vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal );

    commandBuffer.setViewport( m_swapchain.getViewport() );
    commandBuffer.setScissor( m_swapchain.getScissor() );

    commandBuffer.beginRendering( m_swapchain.getExtent(), m_colorImage->getImageView(),
                                  m_swapchain.getImageView( imageIndex ), m_depthBuffer->getImageView() );

    auto currentDescriptorSet{ currentFrame.descriptorSet };
    drawScene( commandBuffer, currentDescriptorSet );
    drawSkybox( commandBuffer, currentDescriptorSet );

    commandBuffer.endRendering();

    commandBuffer.transitionImageLayout( m_swapchain.getImage( imageIndex ), m_swapchain.getFormat(),
                                         vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR );
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

void Engine::drawScene( const ve::GraphicsCommandBuffer currentCommandBuffer,
                        const vk::DescriptorSet currentGlobalSet ) {
    auto draw{ [ &currentCommandBuffer, &currentGlobalSet ]( const auto& renderObject ) {
        currentCommandBuffer.bindPipeline( renderObject.material.pipeline.get() );
        currentCommandBuffer.bindDescriptorSet( renderObject.material.pipeline.getLayout(), currentGlobalSet, 0U );
        currentCommandBuffer.bindDescriptorSet( renderObject.material.pipeline.getLayout(),
                                                renderObject.material.descriptorSet, 1U );
        currentCommandBuffer.bindIndexBuffer( renderObject.indexBuffer );

        const ve::PushConstants pushConstants{ .worldMatrix{ renderObject.transform },
                                               .vertexBufferAddress{ renderObject.vertexBufferAddress } };
        currentCommandBuffer.pushConstants( renderObject.material.pipeline.getLayout(),
                                            vk::ShaderStageFlagBits::eVertex, pushConstants );
        currentCommandBuffer.drawIndices( renderObject.firstIndex, renderObject.indexCount );
    } };

    std::ranges::for_each( m_mainRenderContext.opaqueSurfaces,
                           [ &draw ]( const auto& renderObject ) { draw( renderObject ); } );
    std::ranges::for_each( m_mainRenderContext.transparentSurfaces,
                           [ &draw ]( const auto& renderObject ) { draw( renderObject ); } );
}

void Engine::drawSkybox( const ve::GraphicsCommandBuffer currentCommandBuffer,
                         const vk::DescriptorSet currentGlobalSet ) {
    currentCommandBuffer.bindPipeline( m_skyboxPipeline->get() );
    currentCommandBuffer.bindDescriptorSet( m_skyboxPipelineLayout->get(), currentGlobalSet, 0U );
    currentCommandBuffer.bindDescriptorSet( m_skyboxPipelineLayout->get(), m_skyboxDescriptorSet, 1U );
    currentCommandBuffer.drawVertices( 0U, 36U );
}

void Engine::drawHDRI( const ve::GraphicsCommandBuffer currentCommandBuffer,
                       const vk::DescriptorSet currentGlobalSet ) {
    currentCommandBuffer.bindPipeline( m_hdrPipeline->get() );
    currentCommandBuffer.bindDescriptorSet( m_hdrPipelineLayout->get(), m_hdrDescriptorSet, 0U );
    currentCommandBuffer.drawVertices( 0U, 36U );
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

void Engine::createColorResources() {
    static constexpr uint32_t multisampleBufferMipmapLevel{ 1U };
    m_colorImage.emplace( m_memoryAllocator, m_logicalDevice, m_swapchain.getExtent(), m_swapchain.getFormat(),
                          vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                          vk::ImageAspectFlagBits::eColor, multisampleBufferMipmapLevel,
                          m_physicalDevice.getMaxSamplesCount() );
}

void Engine::createDepthBuffer() {
    static constexpr uint32_t depthMipmapLevel{ 1U };
    m_depthBuffer.emplace( m_memoryAllocator, m_logicalDevice, m_swapchain.getExtent(), vk::Format::eD32Sfloat,
                           vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth,
                           depthMipmapLevel, m_physicalDevice.getMaxSamplesCount() );
}

void Engine::preparePipelines() {
    m_descriptorSetLayout.addBinding( 0U, vk::DescriptorType::eUniformBuffer,
                                      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment );
    m_descriptorSetLayout.create();
    m_metalRough.buildPipelines( m_descriptorSetLayout );
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

void Engine::prepareDefaultTexture() {
    int width;
    int height;
    int channels;
    const auto fullTexturePath{ cfg::directory::assets / "white.png" };

    stbi_uc *pixels{ stbi_load( fullTexturePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha ) };

    if ( !pixels )
        throw std::runtime_error( "failed to load texture image" );

    const vk::DeviceSize bufferSize{ static_cast< vk::DeviceSize >( width ) * height * 4 };
    ve::StagingBuffer stagingBuffer{ m_memoryAllocator, bufferSize };
    memcpy( stagingBuffer.getMappedMemory(), pixels, bufferSize );

    const vk::Extent2D imageExtent{ static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) };
    const uint32_t mipLevels{ static_cast< uint32_t >( std::floor( std::log2( std::max( width, height ) ) ) ) + 1U };
    m_defaultWhiteImage.emplace( m_memoryAllocator, m_logicalDevice, imageExtent, vk::Format::eR8G8B8A8Srgb,
                                 vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                                     vk::ImageUsageFlagBits::eSampled,
                                 vk::ImageAspectFlagBits::eColor, mipLevels );

    immediateSubmit( [ &stagingBuffer, this, mipLevels ]( ve::GraphicsCommandBuffer cmd ) {
        cmd.transitionImageLayout( m_defaultWhiteImage->get(), m_defaultWhiteImage->getFormat(),
                                   vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels );
        cmd.copyBufferToImage( stagingBuffer.get(), m_defaultWhiteImage->get(), m_defaultWhiteImage->getExtent() );
    } );

    generateMipmaps( m_defaultWhiteImage.value(), width, height, mipLevels );
}

void Engine::createDefaultTextureSampler() {
    m_defaultTextureSampler.emplace( m_logicalDevice );
}

void Engine::loadMeshes() {
    const auto sponza{ m_loader.load( cfg::directory::assets / "spheres/MetalRoughSpheres.gltf" ) };
    if ( sponza.has_value() )
        m_scene.emplace( "sponza", sponza.value() );
}

void Engine::handleWindowResising() {
    m_swapchain.recreate();
    createColorResources();
    createDepthBuffer();
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

void Engine::updateScene( float deltaTime ) {
    m_mainRenderContext.opaqueSurfaces.clear();
    m_mainRenderContext.transparentSurfaces.clear();

    if ( m_camera != nullptr ) {
        m_camera->update( deltaTime );
        m_sceneData.cameraPosition = m_camera->getPosition();
    }

    const auto& extent{ m_swapchain.getExtent() };
    constexpr float angle{ 45.0F };
    constexpr float nearPlane{ 0.1F };
    constexpr float farPlane{ 1000.0F };

    m_sceneData.model = glm::mat4{ 1.0F } * glm::scale( glm::mat4{ 1.0F }, glm::vec3{ 3.0F, 3.0F, 3.0F } );

    if ( m_camera != nullptr )
        m_sceneData.view = m_camera->getViewMartix();

    m_sceneData.projection = glm::perspective(
        glm::radians( angle ), static_cast< float >( extent.width ) / extent.height, nearPlane, farPlane );

    m_sceneData.projection[ 1 ][ 1 ] *= -1;

    std::ranges::for_each( m_scene | std::views::values,
                           [ this ]( auto& object ) { object->render( glm::mat4{ 1.0F }, m_mainRenderContext ); } );
}

void Engine::initDefaultData() {
    using Constants = ve::gltf::MetalicRoughness::Constants;

    m_constantsBuffer.emplace( m_memoryAllocator, sizeof( Constants ) );
    Constants *constants{ static_cast< Constants * >( m_constantsBuffer->getMappedMemory() ) };
    constants->colorFactors            = glm::vec4{ 1.0F, 1.0F, 1.0F, 1.0F };
    constants->metalicRoughnessFactors = glm::vec4{ 1.0F, 0.0F, 0.0F, 0.0F };

    static constexpr uint32_t offset{ 0U };

    const vk::ImageView defaultImageView{ m_defaultWhiteImage->getImageView() };
    const vk::Sampler defaultSampler{ m_defaultTextureSampler->get() };

    m_defaultResources.colorImageView            = defaultImageView;
    m_defaultResources.normalMapView             = defaultImageView;
    m_defaultResources.metalicRoughnessImageView = defaultImageView;
    m_defaultResources.colorSampler              = defaultSampler;
    m_defaultResources.normalSampler             = defaultSampler;
    m_defaultResources.metalicRoughnessSampler   = defaultSampler;
    m_defaultResources.dataBuffer                = m_constantsBuffer->get();
    m_defaultResources.dataBufferOffset          = 0U;

    m_defaultMaterial.emplace(
        m_metalRough.writeMaterial( ve::Material::Type::eMainColor, m_defaultResources, m_globalDescriptorAllocator ) );
}

ve::Image Engine::createImage( void *data, const vk::Extent2D size, const vk::Format format,
                               const vk::ImageUsageFlags usage, const uint32_t mipLevels ) {
    const auto& [ width, height ]{ size };
    const vk::DeviceSize bufferSize{ static_cast< vk::DeviceSize >( width ) * height * 4 };
    ve::StagingBuffer stagingBuffer{ m_memoryAllocator, bufferSize };
    memcpy( stagingBuffer.getMappedMemory(), data, bufferSize );

    const vk::Extent2D imageExtent{ static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) };
    ve::Image image{ m_memoryAllocator, m_logicalDevice, imageExtent, format, usage, vk::ImageAspectFlagBits::eColor,
                     mipLevels };

    immediateSubmit( [ this, &stagingBuffer, &image, mipLevels, size ]( ve::GraphicsCommandBuffer cmd ) {
        cmd.transitionImageLayout( image.get(), image.getFormat(), vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal, mipLevels );
        cmd.copyBufferToImage( stagingBuffer.get(), image.get(), image.getExtent() );
    } );

    generateMipmaps( image, size.width, size.height, mipLevels );

    return image;
}

void Engine::generateMipmaps( const ve::Image& image, const int32_t texWidth, const int32_t texHeight,
                              const uint32_t mipLevels ) {
    const auto formatProperties{ m_physicalDevice.get().getFormatProperties( image.getFormat() ) };
    if ( !( formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear ) ) {
        spdlog::warn( "Image format does not support linear blitting. Mipmapping ommited." );
        return;
    }

    immediateSubmit( [ & ]( ve::GraphicsCommandBuffer cmd ) {
        const auto imageVk{ image.get() };
        const auto bufferVk{ cmd.get() };

        vk::ImageMemoryBarrier barrier{};
        barrier.image                           = imageVk;
        barrier.srcQueueFamilyIndex             = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex             = vk::QueueFamilyIgnored;
        barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0U;
        barrier.subresourceRange.layerCount     = 1U;
        barrier.subresourceRange.levelCount     = 1U;

        int32_t mipWidth{ texWidth };
        int32_t mipHeight{ texHeight };
        vk::Offset3D destinationMipmapSize{};
        static constexpr int offsetStartID{ 0 };
        static constexpr int offsetEndID{ 1 };

        for ( uint32_t mipLevel{ 1 }; mipLevel < mipLevels; ++mipLevel ) {
            barrier.subresourceRange.baseMipLevel = mipLevel - 1U;
            barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask                 = vk::AccessFlagBits::eTransferRead;

            bufferVk.pipelineBarrier( vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                      vk::DependencyFlags{}, nullptr, nullptr, barrier );

            vk::ImageBlit blit{};
            blit.srcOffsets.at( offsetStartID ) = vk::Offset3D{ 0, 0, 0 };
            blit.srcOffsets.at( offsetEndID )   = vk::Offset3D{ mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask      = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel        = mipLevel - 1U;
            blit.srcSubresource.baseArrayLayer  = 0U;
            blit.srcSubresource.layerCount      = 1U;

            destinationMipmapSize =
                vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

            blit.dstOffsets.at( offsetStartID ) = vk::Offset3D{ 0, 0, 0 };
            blit.dstOffsets.at( offsetEndID )   = destinationMipmapSize;
            blit.dstSubresource.aspectMask      = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel        = mipLevel;
            blit.dstSubresource.baseArrayLayer  = 0U;
            blit.dstSubresource.layerCount      = 1U;

            bufferVk.blitImage( imageVk, vk::ImageLayout::eTransferSrcOptimal, imageVk,
                                vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear );

            barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            bufferVk.pipelineBarrier( vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                      vk::DependencyFlags{}, nullptr, nullptr, barrier );

            if ( mipWidth > 1 )
                mipWidth /= 2;
            if ( mipHeight > 1 )
                mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask                 = vk::AccessFlagBits::eShaderRead;

        bufferVk.pipelineBarrier( vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                  vk::DependencyFlags{}, nullptr, nullptr, barrier );
    } );
}

void Engine::prepareSkyboxTexture() {
    // std::array< std::string, 6U > skyboxTexturesNames;
    // std::array< stbi_uc *, 6U > skyboxTextureData;

    // skyboxTexturesNames.at( 0 ) = ( cfg::directory::assets / "skybox/right.jpg" ).string();
    // skyboxTexturesNames.at( 1 ) = ( cfg::directory::assets / "skybox/left.jpg" ).string();
    // skyboxTexturesNames.at( 2 ) = ( cfg::directory::assets / "skybox/top.jpg" ).string();
    // skyboxTexturesNames.at( 3 ) = ( cfg::directory::assets / "skybox/bottom.jpg" ).string();
    // skyboxTexturesNames.at( 4 ) = ( cfg::directory::assets / "skybox/front.jpg" ).string();
    // skyboxTexturesNames.at( 5 ) = ( cfg::directory::assets / "skybox/back.jpg" ).string();

    // int width{}, height{}, nrChannels{};
    // for ( size_t textureID{ 0U }; textureID < 6U; textureID++ ) {
    //     skyboxTextureData.at( textureID ) =
    //         stbi_load( skyboxTexturesNames.at( textureID ).data(), &width, &height, &nrChannels, STBI_rgb_alpha );
    // }

    // const vk::DeviceSize layerSize{ static_cast< vk::DeviceSize >( width ) * height * 4 };
    // const vk::DeviceSize imageSize{ 6U * layerSize };
    // ve::StagingBuffer stagingBuffer{ m_memoryAllocator, imageSize };

    // for ( size_t textureID{ 0U }; textureID < 6U; textureID++ ) {
    //     const vk::DeviceSize memoryAddress{ reinterpret_cast< vk::DeviceSize >( stagingBuffer.getMappedMemory() ) +
    //                                         layerSize * textureID };
    //     memcpy( reinterpret_cast< void * >( memoryAddress ), skyboxTextureData.at( textureID ),
    //             static_cast< size_t >( layerSize ) );
    // }

    // const vk::Extent2D imageExtent{ static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) };
    // m_skyboxImage.emplace( m_memoryAllocator, m_logicalDevice, imageExtent, vk::Format::eR8G8B8A8Srgb,
    //                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    //                        vk::ImageAspectFlagBits::eColor, 1U, vk::SampleCountFlagBits::e1, 6U,
    //                        vk::ImageViewType::eCube );

    // immediateSubmit( [ this, &stagingBuffer, imageSize ]( ve::GraphicsCommandBuffer cmd ) {
    //     cmd.transitionImageLayout( m_skyboxImage->get(), m_skyboxImage->getFormat(), vk::ImageLayout::eUndefined,
    //                                vk::ImageLayout::eTransferDstOptimal, 1U, 6U );
    //     cmd.copyBufferToImage( stagingBuffer.get(), m_skyboxImage->get(), m_skyboxImage->getExtent(), 6U );
    //     cmd.transitionImageLayout( m_skyboxImage->get(), m_skyboxImage->getFormat(),
    //                                vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1U,
    //                                6U );
    // } );

    vk::SamplerCreateInfo info{};
    info.magFilter        = vk::Filter::eLinear;
    info.minFilter        = vk::Filter::eLinear;
    info.addressModeU     = vk::SamplerAddressMode::eRepeat;
    info.addressModeV     = vk::SamplerAddressMode::eRepeat;
    info.addressModeW     = vk::SamplerAddressMode::eRepeat;
    info.compareOp        = vk::CompareOp::eNever;
    info.mipLodBias       = 0.0f;
    info.mipmapMode       = vk::SamplerMipmapMode::eLinear;
    info.minLod           = 0.0f;
    info.maxLod           = 1.0f;
    info.maxAnisotropy    = 4.0f;
    info.anisotropyEnable = vk::True;
    info.borderColor      = vk::BorderColor::eFloatOpaqueWhite;
    m_skyboxSampler.emplace( m_logicalDevice, info );
}

void Engine::createSkybox() {
    prepareSkyboxTexture();

    m_skyboxDescriptorSetLayout.addBinding( 0U, vk::DescriptorType::eCombinedImageSampler,
                                            vk::ShaderStageFlagBits::eFragment );
    m_skyboxDescriptorSetLayout.create();

    const std::array< vk::DescriptorSetLayout, 2U > layoutsVk{ m_descriptorSetLayout.get(),
                                                               m_skyboxDescriptorSetLayout.get() };
    vk::PipelineLayoutCreateInfo skyboxLayoutInfo;
    skyboxLayoutInfo.pSetLayouts            = std::data( layoutsVk );
    skyboxLayoutInfo.setLayoutCount         = std::size( layoutsVk );
    skyboxLayoutInfo.pPushConstantRanges    = nullptr;
    skyboxLayoutInfo.pushConstantRangeCount = 0U;

    m_skyboxPipelineLayout.emplace( m_logicalDevice, skyboxLayoutInfo );

    m_pipelineBuilder.setLayout( m_skyboxPipelineLayout.value() );
    m_pipelineBuilder.setShaders( m_skyboxVertexShader, m_skyboxFragmentShader );
    m_pipelineBuilder.setCullingMode( vk::CullModeFlagBits::eFront );
    m_skyboxPipeline.emplace( m_pipelineBuilder );

    m_skyboxDescriptorSet = m_globalDescriptorAllocator.allocate( m_skyboxDescriptorSetLayout );
    m_descriptorWriter.clear();
    m_descriptorWriter.writeImage( 0U, m_hdrSkybox->getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal,
                                   m_skyboxSampler.value().get(), vk::DescriptorType::eCombinedImageSampler );
    m_descriptorWriter.updateSet( m_skyboxDescriptorSet );
}

void Engine::loadHDRI() {
    int width{};
    int height{};
    int nrComponents{};
    stbi_set_flip_vertically_on_load( true );
    float *hdrData = stbi_loadf( ( cfg::directory::assets / "hdr/newport_loft.hdr" ).string().c_str(), &width, &height,
                                 &nrComponents, 0 );

    if ( hdrData ) {
        static constexpr int fixedComponents = 4;
        std::vector< uint16_t > rgbaData( width * height * fixedComponents );
        for ( int i = 0; i < width * height; i++ ) {
            rgbaData[ i * fixedComponents ]     = glm::packHalf1x16( hdrData[ nrComponents * i ] );     // r
            rgbaData[ i * fixedComponents + 1 ] = glm::packHalf1x16( hdrData[ nrComponents * i + 1 ] ); // g
            rgbaData[ i * fixedComponents + 2 ] = glm::packHalf1x16( hdrData[ nrComponents * i + 2 ] ); // b
            rgbaData[ i * fixedComponents + 3 ] = static_cast< uint16_t >( 1 );                         // b
        }

        const vk::Extent2D imageExtent{ static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) };
        m_hdrImage.emplace( m_memoryAllocator, m_logicalDevice, imageExtent, vk::Format::eR16G16B16A16Sfloat,
                            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                            vk::ImageAspectFlagBits::eColor, 1U, vk::SampleCountFlagBits::e1, 1U,
                            vk::ImageViewType::e2D );

        const vk::DeviceSize layerSize{ static_cast< vk::DeviceSize >( width ) * height * fixedComponents *
                                        sizeof( uint16_t ) };
        ve::StagingBuffer stagingBuffer{ m_memoryAllocator, layerSize };

        const vk::DeviceSize memoryAddress{ reinterpret_cast< vk::DeviceSize >( stagingBuffer.getMappedMemory() ) };
        memcpy( reinterpret_cast< void * >( memoryAddress ), std::data( rgbaData ),
                static_cast< size_t >( layerSize ) );

        immediateSubmit( [ this, &stagingBuffer, layerSize ]( ve::GraphicsCommandBuffer cmd ) {
            cmd.transitionImageLayout( m_hdrImage->get(), m_hdrImage->getFormat(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eTransferDstOptimal, 1U, 1U );
            cmd.copyBufferToImage( stagingBuffer.get(), m_hdrImage->get(), m_hdrImage->getExtent(), 1U );
            cmd.transitionImageLayout( m_hdrImage->get(), m_hdrImage->getFormat(), vk::ImageLayout::eTransferDstOptimal,
                                       vk::ImageLayout::eShaderReadOnlyOptimal, 1U, 1U );
        } );

        vk::SamplerCreateInfo info{};
        info.magFilter        = vk::Filter::eLinear;
        info.minFilter        = vk::Filter::eLinear;
        info.addressModeU     = vk::SamplerAddressMode::eClampToEdge;
        info.addressModeV     = vk::SamplerAddressMode::eClampToEdge;
        info.compareOp        = vk::CompareOp::eNever;
        info.mipLodBias       = 0.0f;
        info.mipmapMode       = vk::SamplerMipmapMode::eLinear;
        info.minLod           = 0.0f;
        info.maxLod           = 1.0f;
        info.maxAnisotropy    = 4.0f;
        info.anisotropyEnable = vk::True;
        info.borderColor      = vk::BorderColor::eFloatOpaqueWhite;

        m_hdrSampler.emplace( m_logicalDevice, info );

        stbi_image_free( hdrData );
    } else {
        spdlog::warn( "failed to load hdri" );
    }

    m_hdrUniformBuf.emplace( m_memoryAllocator, sizeof( glm::mat4 ) * 7 );
    glm::mat4 captureProjection = glm::perspectiveLH( glm::radians( 90.0f ), 1.0f, 0.1f, 10.0f );
    glm::mat4 captureViews[]    = {
        glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( -1.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ),
        glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ),
        glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, -1.0f ) ),
        glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) ),
        glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ),
        glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, -1.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ) };

    ve::StagingBuffer st{ m_memoryAllocator, m_hdrUniformBuf->size() };
    memcpy( st.getMappedMemory(), &captureProjection, sizeof( glm::mat4 ) );
    memcpy( static_cast< std::byte * >( st.getMappedMemory() ) + sizeof( glm::mat4 ), &captureViews,
            sizeof( captureViews ) );

    auto logicalDeviceVk{ m_logicalDevice.get() };
    auto commandBufferVk{ m_transferCommandBuffer.get() };
    logicalDeviceVk.resetFences( m_immediateSubmitFence.get() );
    m_transferCommandBuffer.reset();

    m_transferCommandBuffer.begin();

    m_transferCommandBuffer.copyBuffer( 0, 0, m_hdrUniformBuf->size(), st.get(), m_hdrUniformBuf->get() );

    m_transferCommandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.sType              = vk::StructureType::eSubmitInfo;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers    = &commandBufferVk;

    const auto transferQueue{ m_logicalDevice.getQueue( ve::QueueType::eTransfer ) };
    transferQueue.submit( submitInfo, m_immediateSubmitFence.get() );
    [[maybe_unused]] const auto result{
        logicalDeviceVk.waitForFences( m_immediateSubmitFence.get(), g_waitForAllFences, g_timeoutOff ) };

    m_hdrDescriptorSetLayout.addBinding( 0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex );
    m_hdrDescriptorSetLayout.addBinding( 1U, vk::DescriptorType::eCombinedImageSampler,
                                         vk::ShaderStageFlagBits::eFragment );
    m_hdrDescriptorSetLayout.create();

    const std::array< vk::DescriptorSetLayout, 1U > layoutsVk{ m_hdrDescriptorSetLayout.get() };
    vk::PipelineLayoutCreateInfo hdrLayoutInfo;
    hdrLayoutInfo.pSetLayouts            = std::data( layoutsVk );
    hdrLayoutInfo.setLayoutCount         = std::size( layoutsVk );
    hdrLayoutInfo.pPushConstantRanges    = nullptr;
    hdrLayoutInfo.pushConstantRangeCount = 0U;

    m_hdrPipelineLayout.emplace( m_logicalDevice, hdrLayoutInfo );

    ve::PipelineBuilder builder{ m_logicalDevice };

    builder.setLayout( m_hdrPipelineLayout.value() );
    builder.setShaders( m_hdrVertexShader, m_hdrFragmentShader );
    builder.setCullingMode( vk::CullModeFlagBits::eFront );
    builder.setColorFormat( m_hdrImage->getFormat() );
    builder.setSamplesCount( vk::SampleCountFlagBits::e1 );
    builder.setViewMask( 0b111111 );
    m_hdrPipeline.emplace( builder );

    m_hdrDescriptorSet = m_globalDescriptorAllocator.allocate( m_hdrDescriptorSetLayout );
    m_descriptorWriter.clear();
    m_descriptorWriter.writeBuffer( 0U, m_hdrUniformBuf->get(), 7 * sizeof( glm::mat4 ), 0,
                                    vk::DescriptorType::eUniformBuffer );
    m_descriptorWriter.writeImage( 1U, m_hdrImage->getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal,
                                   m_hdrSampler.value().get(), vk::DescriptorType::eCombinedImageSampler );
    m_descriptorWriter.updateSet( m_hdrDescriptorSet );
}

void Engine::renderHDRSkybox() {
    m_hdrSkybox.emplace(
        m_memoryAllocator, m_logicalDevice, vk::Extent2D{ 512u, 512u }, vk::Format::eR16G16B16A16Sfloat,
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlagBits::eColor,
        1U, vk::SampleCountFlagBits::e1, 6U, vk::ImageViewType::eCube );

    auto cmd = m_graphicsCommandPool.createCommandBuffers();
    cmd.begin();

    cmd.setScissor( vk::Rect2D{ vk::Offset2D{ 0, 0 }, vk::Extent2D{ 512u, 512u } } );
    cmd.setViewport( vk::Viewport{ 0, 0, 512, 512, 0, 0 } );

    cmd.transitionImageLayout( m_hdrSkybox->get(), m_hdrSkybox->getFormat(), vk::ImageLayout::eUndefined,
                               vk::ImageLayout::eColorAttachmentOptimal, 1, 6 );

    vk::RenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.pNext              = nullptr;
    colorAttachment.imageView          = m_hdrSkybox->getImageView();
    colorAttachment.imageLayout        = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.resolveImageView   = nullptr;
    colorAttachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.resolveMode        = vk::ResolveModeFlagBits::eNone;
    colorAttachment.loadOp             = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp            = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue         = vk::ClearValue{};

    static constexpr vk::Offset2D defaultOffset{ 0, 0 };
    vk::RenderingInfoKHR renderingInfo{};
    renderingInfo.viewMask             = 0b111111;
    renderingInfo.layerCount           = 6U;
    renderingInfo.colorAttachmentCount = 1U;
    renderingInfo.renderArea           = vk::Rect2D{ defaultOffset, vk::Extent2D{ 512u, 512u } };
    renderingInfo.pColorAttachments    = &colorAttachment;

    cmd.get().beginRendering( renderingInfo );

    auto set = m_globalDescriptorAllocator.allocate( m_descriptorSetLayout );
    drawHDRI( cmd, set );

    cmd.endRendering();

    cmd.transitionImageLayout( m_hdrSkybox->get(), m_hdrSkybox->getFormat(), vk::ImageLayout::eColorAttachmentOptimal,
                               vk::ImageLayout::eShaderReadOnlyOptimal, 1, 6 );

    cmd.end();

    auto cmdVk = cmd.get();

    static constexpr vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::SubmitInfo submitInfo{};
    submitInfo.sType                = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount   = 0U;
    submitInfo.pWaitSemaphores      = nullptr;
    submitInfo.pWaitDstStageMask    = &waitStage;
    submitInfo.commandBufferCount   = 1U;
    submitInfo.pCommandBuffers      = &cmdVk;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pSignalSemaphores    = nullptr;

    const auto graphicsQueue{ m_logicalDevice.getQueue( ve::QueueType::eGraphics ) };
    graphicsQueue.submit( submitInfo, nullptr );
    graphicsQueue.waitIdle();
}

} // namespace ve
