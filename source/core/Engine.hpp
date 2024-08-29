#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"

namespace ve {

class Engine {
public:
    Engine();
    ~Engine();

    void run();
    void render();

private:
    ve::VulkanInstance m_vulkanInstance{};
    ve::Window m_window;
    ve::PhysicalDevice m_physicalDevice;
    ve::LogicalDevice m_logicalDevice;
    ve::Swapchain m_swapchain;
    ve::Pipeline m_pipeline;
    ve::CommandPool m_graphicsCommandPool;
    ve::CommandBuffer m_commandBuffer;

    vk::Semaphore m_imageAvailableSemaphore{};
    vk::Semaphore m_renderFinishedSemaphore{};
    vk::Fence m_inFlightFence{};

    void createSyncObjects();
};

} // namespace ve
