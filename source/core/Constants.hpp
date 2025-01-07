#pragma once

#include <vulkan/vulkan.hpp>

#include <limits>
#include <cstdint>

namespace {

inline constexpr std::uint64_t g_timeoutOff{ std::numeric_limits< std::uint64_t >::max() };
inline constexpr std::uint32_t g_maxFramesInFlight{ 2U };
inline constexpr vk::Bool32 g_waitForAllFences{ vk::True };

} // namespace
