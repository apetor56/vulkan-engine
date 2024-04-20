#pragma once

#include "window.hpp"
#include "vulkan_instance.hpp"
#include "physical_device.hpp"
#include "logical_device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include "command_buffer.hpp"

#include <memory>

namespace VE {

class Application {
public:
    Application();

    ~Application();

    void run();

    void render();

private:
    std::shared_ptr<VulkanInstance> m_vulkanInstance;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<PhysicalDevice> m_physicalDevice;
    std::shared_ptr<LogicalDevice> m_logicalDevice;
    std::shared_ptr<Swapchain> m_swapchain;
    std::shared_ptr<Pipeline> m_pipeline;
    std::shared_ptr<CommandBuffer> m_commandBuffer;

    VkSemaphore m_imageAvailableSemapore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;

    void createSyncObjects();
};

} // namespace VE
