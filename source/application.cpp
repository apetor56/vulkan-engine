#include "application.hpp"
#include "config.hpp"
#include <chrono>
#include <thread>

namespace VE {

Application::Application() : m_vulkanInstance { std::make_shared<VulkanInstance>() },
                             m_window{ std::make_shared<Window>(cfg::window::width, cfg::window::height, "example", m_vulkanInstance) },
                             m_physicalDevice { std::make_shared<PhysicalDevice>(m_vulkanInstance, m_window) },
                             m_logicalDevice { std::make_shared<LogicalDevice>(m_physicalDevice, m_window) },
                             m_swapchain { std::make_shared<Swapchain>(m_physicalDevice, m_logicalDevice, m_window) }
                            {}

void Application::run() {
    while(m_window->shouldClose() == false) {
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

}