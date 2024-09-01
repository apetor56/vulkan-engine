#include "Engine.hpp"
#include "Config.hpp"

#include <limits>

namespace ve {

Engine::Engine()
    : m_window{ WindowInfo{ cfg::window::width, cfg::window::height, "example" }, m_vulkanInstance },
      m_physicalDevice{ m_vulkanInstance, m_window },
      m_logicalDevice{ m_physicalDevice },
      m_swapchain{ m_physicalDevice, m_logicalDevice, m_window },
      m_pipeline{ m_logicalDevice, m_swapchain },
      m_graphicsCommandPool{ m_logicalDevice, m_swapchain, m_pipeline },
      m_commandBuffers{ m_graphicsCommandPool.createCommandBuffers( s_maxFramesInFlight ) } {
    createSyncObjects();
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
}

void Engine::run() {
    while ( m_window.shouldClose() == GLFW_FALSE ) {
        glfwPollEvents();
        render();
    }
    m_logicalDevice.getHandler().waitIdle();
}

void Engine::render() {
    const auto imageIndex{ acquireNextImage() };
    if ( !imageIndex.has_value() )
        return;

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
}

std::optional< std::uint32_t > Engine::acquireNextImage() {
    static constexpr auto waitForAllFences{ vk::True };
    m_logicalDevice.getHandler().waitForFences( m_inFlightFences.at( m_currentFrame ), waitForAllFences, s_timeoutOff );

    try {
        const auto [ result, imageIndex ]{ m_logicalDevice.getHandler().acquireNextImageKHR(
            m_swapchain.getHandler(), s_timeoutOff, m_imageAvailableSemaphores.at( m_currentFrame ) ) };

        if ( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR )
            throw std::runtime_error( "failed to acquire swapchain image" );

        return imageIndex;
    }
    catch ( const vk::OutOfDateKHRError& exception ) {
        m_swapchain.recreate();
        return std::nullopt;
    }
}

void Engine::draw( const std::uint32_t imageIndex ) {
    m_logicalDevice.getHandler().resetFences( m_inFlightFences.at( m_currentFrame ) );

    auto& commandBuffer{ m_commandBuffers.at( m_currentFrame ) };
    const auto commandBufferHandler{ commandBuffer.getHandler() };
    const auto renderFinishedSemaphore{ m_renderFinishedSemaphores.at( m_currentFrame ) };

    commandBuffer.reset();
    commandBuffer.record( imageIndex );

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

    const auto graphicsQueue{ m_logicalDevice.getGraphicsQueue() };
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

    const auto presentationQueue{ m_logicalDevice.getPresentationQueue() };
    try {
        const auto presentResult{ presentationQueue.presentKHR( presentInfo ) };
        if ( presentResult == vk::Result::eSuboptimalKHR || m_window.isResized() )
            m_swapchain.recreate();
    }
    catch ( const vk::OutOfDateKHRError& exception ) {
        m_swapchain.recreate();
    }
}

} // namespace ve
