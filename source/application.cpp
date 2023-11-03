#include "application.hpp"
#include "config.hpp"
#include <chrono>
#include <thread>

namespace VE {

Application::Application() : m_vulkanInstance { std::make_shared<VulkanInstance>() },
                             m_window{ std::make_shared<Window>(cfg::window::width, cfg::window::height, "example", m_vulkanInstance) },
                             m_physicalDevice { std::make_shared<PhysicalDevice>(m_vulkanInstance, m_window) },
                             m_logicalDevice { std::make_shared<LogicalDevice>(m_physicalDevice, m_window) },
                             m_swapchain { std::make_shared<Swapchain>(m_physicalDevice, m_logicalDevice, m_window) },
                             m_pipeline { std::make_shared<Pipeline>(m_logicalDevice, m_swapchain) },
                             m_commandBuffer { std::make_shared<CommandBuffer>(m_physicalDevice, m_logicalDevice, m_swapchain, m_pipeline) }
                            {
    createSyncObjects();
}

Application::~Application() {
    const auto logicalDeviceHandle { m_logicalDevice->getHandle() };
    vkDestroySemaphore(logicalDeviceHandle, m_imageAvailableSemapore, nullptr);
    vkDestroySemaphore(logicalDeviceHandle, m_renderFinishedSemaphore, nullptr);
    vkDestroyFence(logicalDeviceHandle, m_inFlightFence, nullptr);
}

void Application::run() {
    while(m_window->shouldClose() == false) {
        glfwPollEvents();
        render();

        vkDeviceWaitIdle(m_logicalDevice->getHandle());

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void Application::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if(vkCreateSemaphore(m_logicalDevice->getHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemapore) != VK_SUCCESS ||
       vkCreateSemaphore(m_logicalDevice->getHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
       vkCreateFence(m_logicalDevice->getHandle(), &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create semaphore or fence sync object(s)");
    }
}

void Application::render() {
    const auto logicalDeviceHandle { m_logicalDevice->getHandle() };
    const auto swapchainHandle { m_swapchain->getHandle() };
    const auto commadBufferHandle { m_commandBuffer->getHandle() };
    
    vkWaitForFences(logicalDeviceHandle, 1u, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(logicalDeviceHandle, 1u, &m_inFlightFence);

    uint32_t imageIndex{};
    vkAcquireNextImageKHR(logicalDeviceHandle, swapchainHandle, UINT64_MAX, m_imageAvailableSemapore, VK_NULL_HANDLE, &imageIndex);

    m_commandBuffer->reset();
    m_commandBuffer->record(imageIndex);

    const std::vector<VkPipelineStageFlags> waitStages {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1u;
    submitInfo.pWaitSemaphores    = &m_imageAvailableSemapore;
    submitInfo.pWaitDstStageMask  = waitStages.data();

    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers    = &commadBufferHandle;

    submitInfo.signalSemaphoreCount = 1u;
    submitInfo.pSignalSemaphores    = &m_renderFinishedSemaphore;

    if(vkQueueSubmit(m_logicalDevice->getGraphicsQueue(), 1u, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1u;
    presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphore;

    presentInfo.swapchainCount = 1u;
    presentInfo.pSwapchains    = &swapchainHandle;
    presentInfo.pImageIndices  = &imageIndex;

    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(m_logicalDevice->getPresentationQueue(), &presentInfo);
}

}