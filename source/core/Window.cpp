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
    m_vulkanInstance.get().destroySurfaceKHR( m_surface );
}

void Window::init() {
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

    auto& [ width, height, name ]{ m_windowInfo };
    m_windowHandler = glfwCreateWindow( width, height, name.c_str(), nullptr, nullptr );

    glfwSetWindowUserPointer( m_windowHandler, this );
    glfwSetFramebufferSizeCallback( m_windowHandler, framebufferResizeCallback );
}

void Window::framebufferResizeCallback( GLFWwindow *windowHandler, int width, int height ) {
    auto window{ reinterpret_cast< ve::Window * >( glfwGetWindowUserPointer( windowHandler ) ) };
    window->setLogicalSize( width, height );
    window->m_isResized = true;
}

void Window::createSurface() {
    if ( glfwCreateWindowSurface( static_cast< VkInstance >( m_vulkanInstance.get() ), m_windowHandler, nullptr,
                                  &m_surface ) != VK_SUCCESS )
        throw std::runtime_error( "failed to create window surface" );
}

int Window::shouldClose() const {
    return glfwWindowShouldClose( m_windowHandler );
}

} // namespace ve
