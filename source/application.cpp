#include "application.hpp"
#include "config.hpp"

namespace VE {

Application::Application() : m_window{cfg::window::width, cfg::window::height, "example"},
                             m_pipeline{cfg::shader::vertShaderBinary, cfg::shader::fragShaderBinary} {}

void Application::run() {
    while(m_window.shouldClose() == false) {
        glfwPollEvents();
    }
}

}