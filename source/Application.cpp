#include "Application.hpp"
#include "core/Config.hpp"

#include <chrono>
#include <thread>

namespace ve {

void Application::run() {
    m_graphicsEngine.run();
}

} // namespace ve
