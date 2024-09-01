#pragma once

#include "VulkanInstance.hpp"

#include <GLFW/glfw3.h>

#include <string>

namespace ve {

struct WindowInfo {
    int width{};
    int height{};
    std::string name{};
};

class Window {
public:
    Window( WindowInfo windowInfo, const ve::VulkanInstance& instance );
    ~Window();

    Window( const Window& other ) = delete;
    Window( Window&& other )      = delete;

    Window& operator=( const Window& other ) = delete;
    Window& operator=( Window&& other )      = delete;

    int shouldClose() const;
    GLFWwindow *getHandler() const noexcept;
    VkSurfaceKHR getSurface() const noexcept;

    void setResizedFlag( bool value ) noexcept;
    void setSize( int width, int height ) noexcept;
    bool isResized() const noexcept;

private:
    WindowInfo m_windowInfo{};
    const ve::VulkanInstance& m_vulkanInstance;
    GLFWwindow *m_windowHandler{};
    VkSurfaceKHR m_surface{};
    bool m_isResized{ false };

    void init();
    void createSurface();
    static void framebufferResizeCallback( GLFWwindow *windowHandler, int width, int height );
};

} // namespace ve
