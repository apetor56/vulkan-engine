#include "Application.hpp"
#include "core/Config.hpp"

#include <chrono>
#include <thread>

namespace ve {

Application::Application()
    : m_window{ WindowInfo{ cfg::window::width, cfg::window::height, "example" }, m_vulkanInstance },
      m_physicalDevice{ m_vulkanInstance, m_window },
      m_logicalDevice{ m_physicalDevice },
      m_swapchain{ m_physicalDevice, m_logicalDevice, m_window },
      m_pipeline{ m_logicalDevice, m_swapchain },
      m_commandBuffer{ m_physicalDevice, m_logicalDevice, m_swapchain, m_pipeline } {
    createSyncObjects();
}

Application::~Application() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    logicalDeviceHandler.destroySemaphore( m_imageAvailableSemaphore );
    logicalDeviceHandler.destroySemaphore( m_renderFinishedSemaphore );
    logicalDeviceHandler.destroyFence( m_inFlightFence );
}

void Application::run() {
    while ( m_window.shouldClose() == GLFW_FALSE ) {
        glfwPollEvents();
        render();

        vkDeviceWaitIdle( m_logicalDevice.getHandler() );

        std::this_thread::sleep_for( std::chrono::milliseconds( 33 ) );
    }
}

void Application::createSyncObjects() {
    vk::SemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    m_imageAvailableSemaphore = logicalDeviceHandler.createSemaphore( semaphoreInfo );
    m_renderFinishedSemaphore = logicalDeviceHandler.createSemaphore( semaphoreInfo );
    m_inFlightFence           = logicalDeviceHandler.createFence( fenceInfo );
}

void Application::render() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    const auto swapchainHandler{ m_swapchain.getHandler() };
    const auto commadBufferHandler{ m_commandBuffer.getHandler() };

    logicalDeviceHandler.waitForFences( m_inFlightFence, vk::True, UINT64_MAX );
    logicalDeviceHandler.resetFences( m_inFlightFence );

    const std::uint32_t imageIndex{
        logicalDeviceHandler.acquireNextImageKHR( swapchainHandler, UINT64_MAX, m_imageAvailableSemaphore ).value };

    m_commandBuffer.reset();
    m_commandBuffer.record( imageIndex );

    const std::vector< vk::PipelineStageFlags > waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submitInfo{};
    submitInfo.sType                = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount   = 1U;
    submitInfo.pWaitSemaphores      = &m_imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask    = waitStages.data();
    submitInfo.commandBufferCount   = 1U;
    submitInfo.pCommandBuffers      = &commadBufferHandler;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores    = &m_renderFinishedSemaphore;

    const auto graphicsQueue{ m_logicalDevice.getGraphicsQueue() };
    graphicsQueue.submit( submitInfo, m_inFlightFence );

    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphore;
    presentInfo.swapchainCount     = 1U;
    presentInfo.pSwapchains        = &swapchainHandler;
    presentInfo.pImageIndices      = &imageIndex;

    const auto presentationQueue{ m_logicalDevice.getPresentationQueue() };
    presentationQueue.presentKHR( presentInfo );
}

} // namespace ve
