#include "Window.hpp"

#include <stdexcept>

namespace ve {

Window::Window( const int width, const int height, std::string_view name, std::shared_ptr< VulkanInstance > instance )
    : m_width{ width }, m_height{ height }, m_name{ name }, m_vulkanInstance{ instance } {
    init();
    createSurface();
}

Window::~Window() {
    glfwDestroyWindow( m_windowHandler );
    vkDestroySurfaceKHR( m_vulkanInstance->get(), m_surface, nullptr );
}

void Window::init() {
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    m_windowHandler = glfwCreateWindow( m_width, m_height, m_name.c_str(), nullptr, nullptr );
}

void Window::createSurface() {
    if ( glfwCreateWindowSurface( m_vulkanInstance->get(), m_windowHandler, nullptr, &m_surface ) != VK_SUCCESS )
        throw std::runtime_error( "failed to create window surface" );
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose( m_windowHandler );
}

GLFWwindow *Window::getWindowHandler() const noexcept {
    return m_windowHandler;
}

VkSurfaceKHR Window::getSurface() const noexcept {
    return m_surface;
}

} // namespace ve
