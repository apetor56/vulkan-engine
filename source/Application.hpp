#pragma once

#include "core/Engine.hpp"

namespace ve {

class Application {
public:
    Application()  = default;
    ~Application() = default;

    Application( const Application& other ) = delete;
    Application( Application&& other )      = delete;

    Application& operator=( const Application& other ) = delete;
    Application& operator=( Application&& other )      = delete;

    void run();

private:
    ve::Engine m_graphicsEngine{};
};

} // namespace ve
