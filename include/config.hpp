#pragma once

namespace cfg {

namespace window {
    inline constexpr int width {800};
    inline constexpr int height {600};
} // namespace window

namespace shader {
    inline const char* vertShaderBinary{"../shaders/simple.vert.spv"};
    inline const char* fragShaderBinary{"../shaders/simple.frag.spv"};
} // namespace shader

} // namespace cfg