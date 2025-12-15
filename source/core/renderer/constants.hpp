#pragma once

#include <vulkan/vulkan.hpp>

#include <limits>

namespace {

inline constexpr uint64_t g_timeoutOff{ std::numeric_limits< uint64_t >::max() };
inline constexpr uint32_t g_maxFramesInFlight{ 2U };
inline constexpr vk::Bool32 g_waitForAllFences{ vk::True };

} // namespace
