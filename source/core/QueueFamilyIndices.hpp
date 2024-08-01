#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

namespace ve {

struct QueueFamilyIndices {
    std::optional< std::uint32_t > graphicsFamilyID;
    std::optional< std::uint32_t > presentFamilyID;

    bool hasRequiredFamilies() const noexcept;
    static QueueFamilyIndices findQueueFamilies( const vk::PhysicalDevice device, const vk::SurfaceKHR surface );
};

} // namespace ve
