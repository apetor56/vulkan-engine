#include "Window.hpp"

#include <stdexcept>

namespace ve {

Window::Window( WindowInfo windowInfo, const ve::VulkanInstance& instance )
    : m_windowInfo{ std::move( windowInfo ) }, m_vulkanInstance{ instance } {
    init();
    createSurface();
}

Window::~Window() {
    glfwDestroyWindow( m_windowHandler );
    vkDestroySurfaceKHR( m_vulkanInstance.get(), m_surface, nullptr );
}

void Window::init() {
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    auto& [ width, height, name ]{ m_windowInfo };
    m_windowHandler = glfwCreateWindow( width, height, name.c_str(), nullptr, nullptr );
}

void Window::createSurface() {
    if ( glfwCreateWindowSurface( m_vulkanInstance.get(), m_windowHandler, nullptr, &m_surface ) != VK_SUCCESS )
        throw std::runtime_error( "failed to create window surface" );
}

int Window::shouldClose() const {
    return glfwWindowShouldClose( m_windowHandler );
}

GLFWwindow *Window::getHandler() const noexcept {
    return m_windowHandler;
}

VkSurfaceKHR Window::getSurface() const noexcept {
    return m_surface;
}

} // namespace ve
