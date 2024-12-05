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

class Window : public utils::NonCopyable,
               public utils::NonMovable {
public:
    Window( WindowInfo windowInfo, const ve::VulkanInstance& instance );
    ~Window();

    int shouldClose() const;
    GLFWwindow *get() const noexcept { return m_windowHandler; }
    VkSurfaceKHR getSurface() const noexcept { return m_surface; }

    void setLogicalSize( int width, int height ) noexcept {
        m_windowInfo.width  = width;
        m_windowInfo.height = height;
    }
    void setResizeFlag( bool resized ) noexcept { m_isResized = resized; }
    bool isResized() const noexcept { return m_isResized; }

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
