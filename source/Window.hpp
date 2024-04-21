#pragma once

#include "VulkanInstance.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <string>
#include <string_view>

namespace ve {

class Window {
public:
    Window( const int width, const int height, std::string_view name, const ve::VulkanInstance& instance );
    ~Window();

    Window( const Window& other ) = delete;
    Window( Window&& other )      = delete;

    Window& operator=( const Window& other ) = delete;
    Window& operator=( Window&& other )      = delete;

    bool shouldClose() const;
    GLFWwindow *getWindowHandler() const noexcept;
    VkSurfaceKHR getSurface() const noexcept;

private:
    const int m_width;
    const int m_height;
    const std::string m_name;
    const ve::VulkanInstance& m_vulkanInstance;
    GLFWwindow *m_windowHandler;
    VkSurfaceKHR m_surface;

    void init();
    void createSurface();
};

} // namespace ve
