#pragma once

#include "window.hpp"
#include "vulkan_instance.hpp"
#include "physical_device.hpp"
#include "logical_device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"

#include <memory>

namespace VE {

class Application {
public:
    Application();

    void run();

private:
    std::shared_ptr<VulkanInstance> m_vulkanInstance;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<PhysicalDevice> m_physicalDevice;
    std::shared_ptr<LogicalDevice> m_logicalDevice;
    std::shared_ptr<Swapchain> m_swapchain;
    std::shared_ptr<Pipeline> m_pipeline;
};

}
