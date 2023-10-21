#pragma once

#include "window.hpp"
#include "vulkan_instance.hpp"
#include "device.hpp"
#include <memory>

namespace VE {

class Application {
public:
    Application();

    void run();

private:
    std::shared_ptr<Window> m_window;
    std::shared_ptr<VulkanInstance> m_vulkanInstance;
    Device m_device;
};

}
