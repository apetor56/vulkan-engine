#pragma once

// TODO: better config system

#include <filesystem>

namespace cfg::debug {

#ifndef NDEBUG
inline constexpr bool areValidationLayersEnabled{ true };
#else
inline constexpr bool areValidationLayersEnabled{ false };
#endif

} // namespace cfg::debug

namespace cfg::window {

inline constexpr int width{ 800 };
inline constexpr int height{ 600 };

} // namespace cfg::window

namespace cfg::directory {

inline const std::filesystem::path shaderBinaries{ SHADER_BINARIES_DIR };
inline const std::filesystem::path assets{ ASSETS_DIR };

} // namespace cfg::directory

namespace cfg::gpu {

inline constexpr uint32_t discreteGpuValue{ 1000U };

} // namespace cfg::gpu

namespace cfg::device {

inline constexpr uint32_t queueFamiliesCount{ 2U };

} // namespace cfg::device
