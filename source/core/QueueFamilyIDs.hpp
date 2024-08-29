#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

namespace ve {

struct QueueFamilyIDs {
    std::optional< std::uint32_t > graphicsFamilyID;
    std::optional< std::uint32_t > presentationFamilyID;

    bool hasRequiredFamilies() const noexcept;
    static QueueFamilyIDs findQueueFamilies( const vk::PhysicalDevice device, const vk::SurfaceKHR surface );
};

} // namespace ve
