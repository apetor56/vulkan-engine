#include "application.hpp"
#include "config.hpp"
#include <chrono>
#include <thread>

namespace VE {

Application::Application() : m_window{ std::make_shared<Window>(cfg::window::width, cfg::window::height, "example") },
                             m_vulkanInstance { std::make_shared<VulkanInstance>() },
                             m_device { m_vulkanInstance, m_window } {}

void Application::run() {
    while(m_window->shouldClose() == false) {
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

}