#pragma once

namespace cfg {

namespace window {
inline constexpr int width{ 800 };
inline constexpr int height{ 600 };
} // namespace window

namespace shader {
// TODO: add filesystem
inline const char *vertShaderBinaryPath{ "./shaders/simple.vert.spv" };
inline const char *fragShaderBinaryPath{ "./shaders/simple.frag.spv" };
} // namespace shader

namespace gpu {
inline constexpr size_t discreteGpuValue{ 1000u };
} // namespace gpu

namespace device {
inline constexpr uint32_t queueFamiliesCount{ 2u };
} // namespace device

} // namespace cfg
