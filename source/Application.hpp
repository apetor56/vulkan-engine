#pragma once

#include "Window.hpp"
#include "VulkanInstance.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"

#include <memory>

namespace ve {

class Application {
public:
    Application();
    ~Application();

    void run();
    void render();

private:
    std::shared_ptr< VulkanInstance > m_vulkanInstance;
    std::shared_ptr< Window > m_window;
    std::shared_ptr< PhysicalDevice > m_physicalDevice;
    std::shared_ptr< LogicalDevice > m_logicalDevice;
    std::shared_ptr< Swapchain > m_swapchain;
    std::shared_ptr< Pipeline > m_pipeline;
    std::shared_ptr< CommandBuffer > m_commandBuffer;

    VkSemaphore m_imageAvailableSemapore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;

    void createSyncObjects();
};

} // namespace ve
