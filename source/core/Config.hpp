#pragma once

// TODO: better config system

#include <filesystem>
#include <cstdint>

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

namespace cfg::shader {

inline const std::filesystem::path vertShaderBinaryPath{ std::filesystem::path{ SHADER_BINARIES_DIR } /
                                                         "simple.vert.spv" };
inline const std::filesystem::path fragShaderBinaryPath{ std::filesystem::path{ SHADER_BINARIES_DIR } /
                                                         "simple.frag.spv" };
} // namespace cfg::shader

namespace cfg::texture {

inline const std::filesystem::path directory{ std::filesystem::path{ TEXTURE_DIR } };

}

namespace cfg::gpu {

inline constexpr std::uint32_t discreteGpuValue{ 1000U };

} // namespace cfg::gpu

namespace cfg::device {

inline constexpr std::uint32_t queueFamiliesCount{ 2U };

} // namespace cfg::device
