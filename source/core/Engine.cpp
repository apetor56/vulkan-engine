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
      m_swapchain{ m_physicalDevice, m_logicalDevice, m_window },
      m_graphicsCommandPool{ m_logicalDevice },
      m_commandBuffers{ m_graphicsCommandPool.createCommandBuffers< s_maxFramesInFlight >() },
      m_immediateBuffer{ m_graphicsCommandPool.createCommandBuffers() },
      m_transferCommandPool{ m_logicalDevice },
      m_transferCommandBuffer{ m_transferCommandPool.createCommandBuffers() },
      m_vertexBuffer{ m_memoryAllocator, sizeof( Vertex ) * std::size( temporaryVertices ) },
      m_indexBuffer{ m_memoryAllocator, sizeof( std::uint32_t ) * std::size( temporaryIndices ) },
      m_descriptorSetLayout{ m_logicalDevice },
      m_descriptorPool{ m_logicalDevice, vk::DescriptorType::eUniformBuffer, s_maxFramesInFlight,
                        s_maxFramesInFlight } {
    createSyncObjects();

    m_descriptorSetLayout.addBinding( 0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex );
    m_descriptorSetLayout.create();
    m_descriptorSets = m_descriptorPool.createDescriptorSets( s_maxFramesInFlight, m_descriptorSetLayout );
    configureDescriptorSets();

    uploadBuffersData( temporaryVertices, temporaryIndices );

    m_pipeline.emplace( m_logicalDevice, m_swapchain, m_descriptorSetLayout );
    prepareTexture();
}

Engine::~Engine() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

    std::ranges::for_each( m_imageAvailableSemaphores, [ logicalDeviceHandler ]( const auto& semaphore ) {
        logicalDeviceHandler.destroySemaphore( semaphore );
    } );
    std::ranges::for_each( m_renderFinishedSemaphores, [ logicalDeviceHandler ]( const auto& semaphore ) {
        logicalDeviceHandler.destroySemaphore( semaphore );
    } );
    std::ranges::for_each( m_inFlightFences, [ logicalDeviceHandler ]( const auto& semaphore ) {
        logicalDeviceHandler.destroyFence( semaphore );
    } );

    logicalDeviceHandler.destroyFence( m_immediateSubmitFence );
}

void Engine::run() {
    while ( m_window.shouldClose() == GLFW_FALSE ) {
        glfwPollEvents();
        render();
    }
    m_logicalDevice.getHandler().waitIdle();
}

void Engine::render() {
    [[maybe_unused]] const auto waitForFencesResult{ m_logicalDevice.getHandler().waitForFences(
        m_inFlightFences.at( m_currentFrame ), g_waitForAllFences, g_timeoutOff ) };

    const auto imageIndex{ acquireNextImage() };
    if ( !imageIndex.has_value() )
        return;

    m_logicalDevice.getHandler().resetFences( m_inFlightFences.at( m_currentFrame ) );

    draw( imageIndex.value() );
    present( imageIndex.value() );

    m_currentFrame = ( m_currentFrame + 1U ) % s_maxFramesInFlight;
}

void Engine::createSyncObjects() {
    vk::SemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    for ( std::uint32_t index{ 0 }; index < s_maxFramesInFlight; index++ ) {
        m_imageAvailableSemaphores.at( index ) = logicalDeviceHandler.createSemaphore( semaphoreInfo );
        m_renderFinishedSemaphores.at( index ) = logicalDeviceHandler.createSemaphore( semaphoreInfo );
        m_inFlightFences.at( index )           = logicalDeviceHandler.createFence( fenceInfo );
    }

    m_immediateSubmitFence = logicalDeviceHandler.createFence( fenceInfo );
}

std::optional< std::uint32_t > Engine::acquireNextImage() {
    try {
        auto [ result, imageIndex ]{ m_logicalDevice.getHandler().acquireNextImageKHR(
            m_swapchain.getHandler(), g_timeoutOff, m_imageAvailableSemaphores.at( m_currentFrame ) ) };

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
    auto& commandBuffer{ m_commandBuffers.at( m_currentFrame ) };
    const auto commandBufferHandler{ commandBuffer.getHandler() };
    const auto renderFinishedSemaphore{ m_renderFinishedSemaphores.at( m_currentFrame ) };

    commandBuffer.reset();
    commandBuffer.begin();
    commandBuffer.beginRenderPass( m_swapchain.getRenderpass(), m_swapchain.getFrambuffer( imageIndex ),
                                   m_swapchain.getExtent() );
    commandBuffer.bindPipeline( m_pipeline->getHandler() );
    commandBuffer.setViewport( m_swapchain.getViewport() );
    commandBuffer.setScissor( m_swapchain.getScissor() );
    commandBuffer.bindVertexBuffer( m_vertexBuffer.getHandler() );
    commandBuffer.bindIndexBuffer( m_indexBuffer.getHandler() );
    commandBuffer.bindDescriptorSet( m_pipeline->getLayout(), m_descriptorSets.at( m_currentFrame ) );
    commandBuffer.drawIndices( m_indexBuffer.size() / sizeof( std::uint32_t ) );
    commandBuffer.endRenderPass();
    commandBuffer.end();

    updateUniformBuffer();

    static constexpr vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::SubmitInfo submitInfo{};
    submitInfo.sType                = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount   = 1U;
    submitInfo.pWaitSemaphores      = &m_imageAvailableSemaphores.at( m_currentFrame );
    submitInfo.pWaitDstStageMask    = &waitStage;
    submitInfo.commandBufferCount   = 1U;
    submitInfo.pCommandBuffers      = &commandBufferHandler;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores    = &renderFinishedSemaphore;

    const auto graphicsQueue{ m_logicalDevice.getQueue( ve::QueueType::eGraphics ) };
    graphicsQueue.submit( submitInfo, m_inFlightFences.at( m_currentFrame ) );
}

void Engine::present( const std::uint32_t imageIndex ) {
    const auto swapchainHandler{ m_swapchain.getHandler() };
    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphores.at( m_currentFrame );
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

void Engine::updateUniformBuffer() {
    using namespace std::chrono;
    static auto start{ high_resolution_clock::now() };
    auto now{ high_resolution_clock::now() };
    milliseconds elapsed{ duration_cast< milliseconds >( now - start ) };

    UniformBufferData data{};

    constexpr glm::vec3 zAxis{ 0.0F, 0.0F, 1.0F };
    data.model = glm::rotate( glm::mat4( 1.0f ), elapsed.count() * glm::radians( 90.0f ) / 1000, zAxis );

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

    memcpy( m_uniformBuffers.at( m_currentFrame ).getMappedMemory(), &data, sizeof( data ) );
}

void Engine::configureDescriptorSets() {
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.offset = 0U;
    bufferInfo.range  = sizeof( UniformBufferData );

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.sType           = vk::StructureType::eWriteDescriptorSet;
    descriptorWrite.dstBinding      = 0U;
    descriptorWrite.dstArrayElement = 0U;
    descriptorWrite.descriptorType  = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1U;
    descriptorWrite.pBufferInfo     = &bufferInfo;

    for ( std::uint32_t index{ 0U }; index < s_maxFramesInFlight; index++ ) {
        bufferInfo.buffer      = m_uniformBuffers.at( index ).getHandler();
        descriptorWrite.dstSet = m_descriptorSets.at( index );
        m_logicalDevice.getHandler().updateDescriptorSets( descriptorWrite, nullptr );
    }
}

void Engine::uploadBuffersData( std::span< Vertex > vertices, std::span< std::uint32_t > indices ) {
    const vk::DeviceSize vertexBufferSize{ std::size( vertices ) * sizeof( Vertex ) };
    const vk::DeviceSize indexBufferSize{ std::size( indices ) * sizeof( std::uint32_t ) };

    StagingBuffer stagingBuffer{ m_memoryAllocator, vertexBufferSize + indexBufferSize };
    void *mappedMemory{ stagingBuffer.getMappedMemory() };
    memcpy( mappedMemory, std::data( vertices ), vertexBufferSize );
    memcpy( static_cast< char * >( mappedMemory ) + vertexBufferSize, std::data( indices ), indexBufferSize );

    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto commandBufferHandler{ m_transferCommandBuffer.getHandler() };
    logicalDeviceHandler.resetFences( m_immediateSubmitFence );
    m_transferCommandBuffer.reset();

    m_transferCommandBuffer.begin();

    constexpr vk::DeviceSize vertexSrcOffset{ 0U };
    constexpr vk::DeviceSize vertexDstOffset{ 0U };
    m_transferCommandBuffer.copyBuffer( vertexSrcOffset, vertexDstOffset, vertexBufferSize, stagingBuffer.getHandler(),
                                        m_vertexBuffer.getHandler() );

    const vk::DeviceSize indexSrcOffset{ vertexBufferSize };
    constexpr vk::DeviceSize indexDstOffset{ 0U };
    m_transferCommandBuffer.copyBuffer( indexSrcOffset, indexDstOffset, indexBufferSize, stagingBuffer.getHandler(),
                                        m_indexBuffer.getHandler() );

    m_transferCommandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.sType              = vk::StructureType::eSubmitInfo;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers    = &commandBufferHandler;

    const auto transferQueue{ m_logicalDevice.getQueue( ve::QueueType::eTransfer ) };
    transferQueue.submit( submitInfo, m_immediateSubmitFence );
    [[maybe_unused]] const auto result{
        logicalDeviceHandler.waitForFences( m_immediateSubmitFence, g_waitForAllFences, g_timeoutOff ) };
}

void Engine::prepareTexture() {
    int width;
    int height;
    int channels;
    const auto fullTexturePath{ cfg::texture::directory / "sample.png" };

    stbi_uc *pixels{ stbi_load( fullTexturePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha ) };

    if ( !pixels )
        throw std::runtime_error( "failed to load texture image" );

    const vk::DeviceSize bufferSize{ static_cast< vk::DeviceSize >( width ) * height * 4 };
    ve::StagingBuffer stagingBuffer{ m_memoryAllocator, bufferSize };
    memcpy( stagingBuffer.getMappedMemory(), pixels, bufferSize );

    const vk::Extent2D imageExtent{ static_cast< std::uint32_t >( width ), static_cast< std::uint32_t >( height ) };
    m_textureImage.emplace( m_memoryAllocator, imageExtent, vk::Format::eR8G8B8A8Srgb,
                            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled );

    immediateSubmit< ve::GraphicsCommandBuffer >( [ &stagingBuffer, this ]( ve::GraphicsCommandBuffer cmd ) {
        cmd.transitionImageBuffer( m_textureImage->get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal );
        cmd.copyBufferToImage( stagingBuffer.getHandler(), m_textureImage->get(), m_textureImage->getExtent() );
        cmd.transitionImageBuffer( m_textureImage->get(), vk::Format::eR8G8B8A8Srgb,
                                   vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal );
    } );
}

} // namespace ve
