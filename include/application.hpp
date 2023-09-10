#pragma once

#include "window.hpp"
#include "device.hpp"
#include <memory>

namespace VE {

class Application {
public:
    Application();

    void run();

private:
    std::shared_ptr<Window> m_window;
    Device m_device;
};

}
