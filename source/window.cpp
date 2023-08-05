#include "window.hpp"

namespace VE {

Window::Window(const int width, const int height, std::string_view name) : m_width{width}, m_height{height}, m_name{name} {
    init();
}

Window::~Window() {}

void Window::init() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_windowHandler = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_windowHandler);
}

} // namespace VE

