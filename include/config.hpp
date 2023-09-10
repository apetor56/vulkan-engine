#pragma once

namespace cfg {

namespace window {
    inline constexpr int width { 800 };
    inline constexpr int height { 600 };
} // namespace m_window

namespace shader {
    inline const char* vertShaderBinary{"../shaders/simple.vert.spv"};
    inline const char* fragShaderBinary{"../shaders/simple.frag.spv"};
} // namespace shader

namespace gpu {
    inline constexpr size_t discreteGpuValue { 1000u };
}

namespace device {
    inline constexpr uint32_t queueFamiliesCount { 2u };
}

} // namespace cfg