#pragma once

#include "vulkan_instance.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <string_view>

namespace VE {

class Window {
public:
    Window(const int width, const int height, std::string_view name, std::shared_ptr<VulkanInstance> instance);

    ~Window();

    Window(const Window& other) = delete;
    Window(Window&& other)      = delete;

    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other)      = delete;

    bool shouldClose() const;

    GLFWwindow* getWindowHandler() const noexcept;

    VkSurfaceKHR getSurface() const noexcept;

private:
    const int m_width;
    const int m_height;
    const std::string m_name;
    std::shared_ptr<VulkanInstance> m_vulkanInstance;
    GLFWwindow* m_windowHandler;
    VkSurfaceKHR m_surface;

    void init();
    void createSurface();
};

} // namespace VE
