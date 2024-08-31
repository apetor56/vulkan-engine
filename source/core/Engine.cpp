#include "Engine.hpp"
#include "Config.hpp"

#include <chrono>
#include <thread>
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
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto swapchainHandler{ m_swapchain.getHandler() };

    auto& currentCommadBuffer{ m_commandBuffers.at( m_currentFrame ) };
    const auto currentCommandBufferHandler{ currentCommadBuffer.getHandler() };
    const auto currentInFlightFence{ m_inFlightFences.at( m_currentFrame ) };
    const auto currentImageAvailableSemaphore{ m_imageAvailableSemaphores.at( m_currentFrame ) };
    const auto currentRenderFinishedSemaphore{ m_renderFinishedSemaphores.at( m_currentFrame ) };

    static constexpr auto waitForAllFences{ vk::True };
    static constexpr auto timeoutOff{ std::numeric_limits< std::uint64_t >::max() };
    logicalDeviceHandler.waitForFences( currentInFlightFence, waitForAllFences, timeoutOff );
    logicalDeviceHandler.resetFences( currentInFlightFence );

    const std::uint32_t imageIndex{
        logicalDeviceHandler.acquireNextImageKHR( swapchainHandler, timeoutOff, currentImageAvailableSemaphore )
            .value };

    currentCommadBuffer.reset();
    currentCommadBuffer.record( imageIndex );

    static constexpr vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submitInfo{};
    submitInfo.sType                = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount   = 1U;
    submitInfo.pWaitSemaphores      = &currentImageAvailableSemaphore;
    submitInfo.pWaitDstStageMask    = &waitStage;
    submitInfo.commandBufferCount   = 1U;
    submitInfo.pCommandBuffers      = &currentCommandBufferHandler;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores    = &currentRenderFinishedSemaphore;

    const auto graphicsQueue{ m_logicalDevice.getGraphicsQueue() };
    graphicsQueue.submit( submitInfo, currentInFlightFence );

    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pWaitSemaphores    = &currentRenderFinishedSemaphore;
    presentInfo.swapchainCount     = 1U;
    presentInfo.pSwapchains        = &swapchainHandler;
    presentInfo.pImageIndices      = &imageIndex;

    const auto presentationQueue{ m_logicalDevice.getPresentationQueue() };
    presentationQueue.presentKHR( presentInfo );

    m_currentFrame = ( m_currentFrame + 1 ) % s_maxFramesInFlight;
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

} // namespace ve
