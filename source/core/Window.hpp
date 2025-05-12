#pragma once

#include "VulkanInstance.hpp"
#include "Camera.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

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
    void setCamera( std::shared_ptr< ve::Camera > camera ) { m_camera = camera; }
    void setResizeFlag( bool resized ) noexcept { m_isResized = resized; }
    bool isResized() const noexcept { return m_isResized; }
    glm::ivec2 getSize() const noexcept { return { m_windowInfo.width, m_windowInfo.height }; }

private:
    WindowInfo m_windowInfo{};
    const ve::VulkanInstance& m_vulkanInstance;
    std::shared_ptr< ve::Camera > m_camera{ nullptr };
    GLFWwindow *m_windowHandler{};
    VkSurfaceKHR m_surface{};
    int m_mouseButton{ GLFW_MOUSE_BUTTON_LEFT };
    int m_mouseAction{ GLFW_RELEASE };
    bool m_isResized{ false };

    void init();
    void createSurface();

    static void framebufferResizeCallback( GLFWwindow *windowHandler, int width, int height );
    static void keyCallback( GLFWwindow *window, int key, int scancode, int action, int mods );
    static void mousePositionCallback( GLFWwindow *window, double xpos, double ypos );
    static void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods );
};

} // namespace ve
