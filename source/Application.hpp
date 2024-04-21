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
    ve::VulkanInstance m_vulkanInstance{};
    ve::Window m_window;
    ve::PhysicalDevice m_physicalDevice;
    ve::LogicalDevice m_logicalDevice;
    ve::Swapchain m_swapchain;
    ve::Pipeline m_pipeline;
    ve::CommandBuffer m_commandBuffer;

    VkSemaphore m_imageAvailableSemapore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;

    void createSyncObjects();
};

} // namespace ve
