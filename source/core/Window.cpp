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
    window->setSize( width, height );
    window->setResizedFlag( true );
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

void Window::setResizedFlag( bool value ) noexcept {
    m_isResized = value;
}

void Window::setSize( int width, int height ) noexcept {
    m_windowInfo.width  = width;
    m_windowInfo.height = height;
}

bool Window::isResized() const noexcept {
    return m_isResized;
}

} // namespace ve
