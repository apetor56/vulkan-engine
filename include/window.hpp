#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string_view>
#include <string>

class GLFWwindow;

namespace VE {

class Window {
public:
    Window(const int width, const int height, std::string_view name);
    ~Window();

    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;

    Window(Window&& other) = delete;
    Window& operator=(Window&& other) = delete;

    void init();
    bool shouldClose() const;

private:
    const int m_width;
    const int m_height;
    const std::string m_name;
    GLFWwindow *m_windowHandler;
};

} // namespace VE
