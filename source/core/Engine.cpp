#include "Engine.hpp"
#include "Config.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <chrono>

namespace ve {

Engine::Engine()
    : m_window{ WindowInfo{ cfg::window::width, cfg::window::height, "example" }, m_vulkanInstance },
      m_physicalDevice{ m_vulkanInstance, m_window },
      m_logicalDevice{ m_physicalDevice },
      m_memoryAllocator{ m_vulkanInstance, m_physicalDevice, m_logicalDevice },
      m_swapchain{ m_logicalDevice, m_window, m_memoryAllocator },
      m_graphicsCommandPool{ m_logicalDevice },
      m_immediateSubmitFence{ m_logicalDevice },
      m_immediateBuffer{ m_graphicsCommandPool.createCommandBuffers() },
      m_transferCommandPool{ m_logicalDevice },
      m_transferCommandBuffer{ m_transferCommandPool.createCommandBuffers() },
      m_descriptorSetLayout{ m_logicalDevice },
      m_loader{ *this, m_memoryAllocator },
      m_descriptorWriter{ m_logicalDevice } {
    prepareDescriptorSetLayout();
    createFramesResoures();
    prepareTexture();
    createTextureSampler();
    configureDescriptorSets();

    m_pipeline.emplace( m_logicalDevice, m_swapchain, m_descriptorSetLayout );

    m_modelMeshes = m_loader.loadMeshes( cfg::assets::directory / "viking_room.obj" );
}

Engine::~Engine() {
    const auto logicalDeviceHandler{ m_logicalDevice.get() };
    logicalDeviceHandler.destroySampler( m_sampler );
}

void Engine::init() {}

void Engine::cleanup() {}

void Engine::run() {
    while ( m_window.shouldClose() == GLFW_FALSE ) {
        glfwPollEvents();

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
        if ( m_currentFrameIt == std::end( m_frames ) )
            m_currentFrameIt = std::begin( m_frames );
    }
    m_logicalDevice.get().waitIdle();
}

std::optional< std::uint32_t > Engine::acquireNextImage() {
    try {
        const auto& currentFrame{ m_currentFrameIt->value() };
        auto [ result, imageIndex ]{ m_logicalDevice.get().acquireNextImageKHR(
            m_swapchain.get(), g_timeoutOff, currentFrame.swapchainSemaphore.get() ) };

        if ( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR )
            throw std::runtime_error( "failed to acquire swapchain image" );

        return imageIndex;
    }
    catch ( const vk::OutOfDateKHRError& ) {
        m_swapchain.recreate();
        return std::nullopt;
    }
}

void Engine::draw( const std::uint32_t imageIndex ) {
    auto& currentFrame{ m_currentFrameIt->value() };

    updateUniformBuffer();

    const auto& commandBuffer{ currentFrame.graphicsCommandBuffer };
    const auto commandBufferHandler{ commandBuffer.get() };
    const auto renderFinishedSemaphore{ currentFrame.renderSemaphore.get() };
    const auto swapchainSemaphore{ currentFrame.swapchainSemaphore.get() };

    commandBuffer.reset();
    commandBuffer.begin();
    commandBuffer.beginRenderPass( m_swapchain.getRenderpass(), m_swapchain.getFrambuffer( imageIndex ),
                                   m_swapchain.getExtent() );
    commandBuffer.bindPipeline( m_pipeline->get() );
    commandBuffer.setViewport( m_swapchain.getViewport() );
    commandBuffer.setScissor( m_swapchain.getScissor() );
    commandBuffer.bindDescriptorSet( m_pipeline->getLayout(), currentFrame.descriptorSet );
    std::ranges::for_each( m_modelMeshes, [ &commandBuffer ]( const auto& meshAsset ) {
        commandBuffer.bindVertexBuffer( meshAsset.buffers.vertexBuffer->get() );
        commandBuffer.bindIndexBuffer( meshAsset.buffers.indexBuffer->get() );
        commandBuffer.drawIndices( meshAsset.buffers.indexBuffer->size() / sizeof( std::uint32_t ) );
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
    submitInfo.pCommandBuffers      = &commandBufferHandler;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores    = &renderFinishedSemaphore;

    const auto graphicsQueue{ m_logicalDevice.getQueue( ve::QueueType::eGraphics ) };
    graphicsQueue.submit( submitInfo, currentFrame.renderFence.get() );
}

void Engine::present( const std::uint32_t imageIndex ) {
    const auto swapchainHandler{ m_swapchain.get() };
    const auto& currentFrame{ m_currentFrameIt->value() };
    const auto renderSemaphore{ currentFrame.renderSemaphore.get() };

    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pWaitSemaphores    = &renderSemaphore;
    presentInfo.swapchainCount     = 1U;
    presentInfo.pSwapchains        = &swapchainHandler;
    presentInfo.pImageIndices      = &imageIndex;

    const auto presentationQueue{ m_logicalDevice.getQueue( ve::QueueType::ePresentation ) };
    try {
        const auto presentResult{ presentationQueue.presentKHR( presentInfo ) };
        if ( presentResult == vk::Result::eSuboptimalKHR || m_window.isResized() )
            m_swapchain.recreate();
    }
    catch ( const vk::OutOfDateKHRError& ) {
        m_swapchain.recreate();
    }
}

void Engine::prepareDescriptorSetLayout() {
    std::vector< vk::DescriptorType > descriptorTypes{ vk::DescriptorType::eUniformBuffer,
                                                       vk::DescriptorType::eCombinedImageSampler };
    m_descriptorSetLayout.addBinding( 0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex );
    m_descriptorSetLayout.addBinding( 1U, vk::DescriptorType::eCombinedImageSampler,
                                      vk::ShaderStageFlagBits::eFragment );
    m_descriptorSetLayout.create();
}

void Engine::createFramesResoures() {
    const auto graphicsCommandBuffers{ m_graphicsCommandPool.createCommandBuffers< g_maxFramesInFlight >() };

    for ( std::uint32_t frameID{ 0U }; frameID < g_maxFramesInFlight; frameID++ )
        m_frames.at( frameID ).emplace( m_logicalDevice, m_memoryAllocator, graphicsCommandBuffers.at( frameID ),
                                        m_descriptorSetLayout );

    m_currentFrameIt = std::begin( m_frames );
}

void Engine::updateUniformBuffer() {
    using namespace std::chrono;
    static auto start{ high_resolution_clock::now() };
    auto now{ high_resolution_clock::now() };
    milliseconds elapsed{ duration_cast< milliseconds >( now - start ) };

    UniformBufferData data{};

    constexpr glm::vec3 zAxis{ 0.0F, 0.0F, 1.0F };
    data.model = glm::scale( glm::mat4{ 1.0F }, glm::vec3{ 1.0F, 1.0F, 1.0F } ) *
                 glm::rotate( glm::mat4( 1.0F ), elapsed.count() * glm::radians( 90.0F ) / 1000, zAxis );

    constexpr glm::vec3 cameraPos{ 2.0F, 2.0F, 2.0F };
    constexpr glm::vec3 centerPos{};
    constexpr glm::vec3 up{ 0.0F, 0.0F, 1.0F };
    data.view = glm::lookAt( cameraPos, centerPos, up );

    static const auto& extent{ m_swapchain.getExtent() };
    constexpr float angle{ 45.0F };
    constexpr float near{ 0.1F };
    constexpr float far{ 20.0F };
    data.projection =
        glm::perspective( glm::radians( angle ), static_cast< float >( extent.width ) / extent.height, near, far );
    data.projection[ 1 ][ 1 ] *= -1;

    const auto& currentFrame{ m_currentFrameIt->value() };
    memcpy( currentFrame.uniformBuffer.getMappedMemory(), &data, sizeof( data ) );
}

void Engine::configureDescriptorSets() {
    std::ranges::for_each( m_frames, [ this ]( auto& frame ) {
        static constexpr std::uint32_t uniformBufferBinding{ 0U };
        m_descriptorWriter.writeBuffer( uniformBufferBinding, frame.value().uniformBuffer.get(),
                                        sizeof( UniformBufferData ), 0U, vk::DescriptorType::eUniformBuffer );

        static constexpr std::uint32_t textureImageBinding{ 1U };
        m_descriptorWriter.writeImage( textureImageBinding, m_textureImage->getImageView(),
                                       vk::ImageLayout::eShaderReadOnlyOptimal, m_sampler,
                                       vk::DescriptorType::eCombinedImageSampler );

        m_descriptorWriter.updateSet( frame.value().descriptorSet );
    } );
}

MeshBuffers Engine::uploadMeshBuffers( std::span< Vertex > vertices, std::span< std::uint32_t > indices ) const {
    MeshBuffers newMeshBuffers;
    newMeshBuffers.vertexBuffer.emplace( m_memoryAllocator, std::size( vertices ) * sizeof( Vertex ) );
    newMeshBuffers.indexBuffer.emplace( m_memoryAllocator, std::size( indices ) * sizeof( std::uint32_t ) );

    const vk::DeviceSize vertexBufferSize{ std::size( vertices ) * sizeof( Vertex ) };
    const vk::DeviceSize indexBufferSize{ std::size( indices ) * sizeof( std::uint32_t ) };

    StagingBuffer stagingBuffer{ m_memoryAllocator, vertexBufferSize + indexBufferSize };
    void *mappedMemory{ stagingBuffer.getMappedMemory() };
    memcpy( mappedMemory, std::data( vertices ), vertexBufferSize );
    memcpy( static_cast< char * >( mappedMemory ) + vertexBufferSize, std::data( indices ), indexBufferSize );

    const auto logicalDeviceHandler{ m_logicalDevice.get() };
    const auto commandBufferHandler{ m_transferCommandBuffer.get() };
    logicalDeviceHandler.resetFences( m_immediateSubmitFence.get() );
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
    submitInfo.pCommandBuffers    = &commandBufferHandler;

    const auto transferQueue{ m_logicalDevice.getQueue( ve::QueueType::eTransfer ) };
    transferQueue.submit( submitInfo, m_immediateSubmitFence.get() );
    [[maybe_unused]] const auto result{
        logicalDeviceHandler.waitForFences( m_immediateSubmitFence.get(), g_waitForAllFences, g_timeoutOff ) };

    return newMeshBuffers;
}

void Engine::prepareTexture() {
    int width;
    int height;
    int channels;
    const auto fullTexturePath{ cfg::assets::directory / "viking_room.png" };

    stbi_uc *pixels{ stbi_load( fullTexturePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha ) };

    if ( !pixels )
        throw std::runtime_error( "failed to load texture image" );

    const vk::DeviceSize bufferSize{ static_cast< vk::DeviceSize >( width ) * height * 4 };
    ve::StagingBuffer stagingBuffer{ m_memoryAllocator, bufferSize };
    memcpy( stagingBuffer.getMappedMemory(), pixels, bufferSize );

    const vk::Extent2D imageExtent{ static_cast< std::uint32_t >( width ), static_cast< std::uint32_t >( height ) };
    m_textureImage.emplace( m_memoryAllocator, m_logicalDevice, imageExtent, vk::Format::eR8G8B8A8Srgb,
                            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                            vk::ImageAspectFlagBits::eColor );

    immediateSubmit< ve::GraphicsCommandBuffer >( [ &stagingBuffer, this ]( ve::GraphicsCommandBuffer cmd ) {
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
    samplerInfo.anisotropyEnable        = VK_TRUE;
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

    m_sampler = m_logicalDevice.get().createSampler( samplerInfo );
}

} // namespace ve
