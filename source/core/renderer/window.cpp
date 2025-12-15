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
    glfwSetKeyCallback( m_windowHandler, keyCallback );
    glfwSetCursorPosCallback( m_windowHandler, mousePositionCallback );
    glfwSetMouseButtonCallback( m_windowHandler, mouseButtonCallback );
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

void Window::keyCallback( GLFWwindow *windowHandler, int key, int scancode, int action, int mods ) {
    auto camera{ static_cast< ve::Window * >( glfwGetWindowUserPointer( windowHandler ) )->m_camera };
    if ( camera == nullptr )
        return;

    camera->processKey( key, action );
}

void Window::mousePositionCallback( GLFWwindow *windowHandler, double xpos, double ypos ) {
    auto window{ reinterpret_cast< ve::Window * >( glfwGetWindowUserPointer( windowHandler ) ) };
    auto camera{ window->m_camera };
    if ( camera == nullptr )
        return;

    camera->processMouse( xpos, ypos, window->m_mouseButton, window->m_mouseAction );
}

void Window::mouseButtonCallback( GLFWwindow *windowHandler, int button, int action, int mods ) {
    auto window{ reinterpret_cast< ve::Window * >( glfwGetWindowUserPointer( windowHandler ) ) };

    window->m_mouseAction = action;
    window->m_mouseButton = button;
}

} // namespace ve
