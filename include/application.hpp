#pragma once

#include "window.hpp"
#include "pipeline.hpp"

namespace VE {

class Application {
public:
    Application();

    void run();

private:
    Window m_window;
    Pipeline m_pipeline;
};

}
