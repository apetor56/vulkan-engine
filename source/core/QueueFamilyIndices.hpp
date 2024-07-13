#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <cstdint>

namespace ve {

struct QueueFamilyIndices {
    std::optional< uint32_t > graphicsFamily;
    std::optional< uint32_t > presentFamily;

    bool isComplete() const;
    static QueueFamilyIndices findQueueFamilies( const VkPhysicalDevice device, const VkSurfaceKHR surface );
};

} // namespace ve