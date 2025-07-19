#include "Engine.hpp"
#include "Config.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>

#include <spdlog/spdlog.h>

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
      m_metalRough{ m_logicalDevice },
      m_globalDescriptorAllocator{ m_logicalDevice, 10U, g_poolSizes },
      m_camera{ std::make_shared< ve::Camera >() } {
    init();
}

void Engine::init() {
    m_window.setCamera( m_camera );
    createDepthBuffer();
    createRenderPass();
    createFramebuffers();
    preparePipelines();
    createFrameResoures();
    prepareDefaultTexture();
    createDefaultTextureSampler();
    configureDescriptorSets();
    initDefaultData();
    loadMeshes();
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

    if ( !m_renderPass.has_value() )
        throw std::runtime_error( "renderpass not initialized" );
    if ( !m_pipeline.has_value() )
        throw std::runtime_error( "pipeline not initialized" );

    commandBuffer.beginRenderPass( m_renderPass->get(), m_framebuffers.at( imageIndex )->get(),
                                   m_swapchain.getExtent() );
    commandBuffer.setViewport( m_swapchain.getViewport() );
    commandBuffer.setScissor( m_swapchain.getScissor() );

    auto currentDescriptorSet{ currentFrame.descriptorSet };

    auto draw{ [ &commandBuffer, &currentDescriptorSet ]( const auto& renderObject ) {
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
    } };

    std::ranges::for_each( m_mainRenderContext.opaqueSurfaces,
                           [ &draw ]( const auto& renderObject ) { draw( renderObject ); } );
    std::ranges::for_each( m_mainRenderContext.transparentSurfaces,
                           [ &draw ]( const auto& renderObject ) { draw( renderObject ); } );

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
    m_descriptorSetLayout.addBinding( 0U, vk::DescriptorType::eUniformBuffer,
                                      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment );
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

    m_metalRough.buildPipelines( m_descriptorSetLayout, m_renderPass.value() );
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
    m_defaultWhiteImage.emplace( m_memoryAllocator, m_logicalDevice, imageExtent, vk::Format::eR8G8B8A8Srgb,
                                 vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                                     vk::ImageUsageFlagBits::eSampled,
                                 vk::ImageAspectFlagBits::eColor );

    immediateSubmit( [ &stagingBuffer, this ]( ve::GraphicsCommandBuffer cmd ) {
        cmd.transitionImageBuffer( m_defaultWhiteImage->get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal );
        cmd.copyBufferToImage( stagingBuffer.get(), m_defaultWhiteImage->get(), m_defaultWhiteImage->getExtent() );
    } );

    const uint32_t mipLevels{ static_cast< uint32_t >( std::floor( std::log2( std::max( width, height ) ) ) ) + 1U };
    generateMipmaps( m_defaultWhiteImage.value(), width, height, mipLevels );
}

void Engine::createDefaultTextureSampler() {
    m_defaultTextureSampler.emplace( m_logicalDevice );
}

void Engine::loadMeshes() {
    const auto spheres{ m_loader.load( cfg::directory::assets / "sponza/Sponza.gltf" ) };

    if ( spheres.has_value() )
        m_scene.emplace( "spheres", spheres.value() );
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

    m_sceneData.model = glm::mat4{ 1.0F };

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
        cmd.transitionImageBuffer( image.get(), image.getFormat(), vk::ImageLayout::eUndefined,
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

} // namespace ve
