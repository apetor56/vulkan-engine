#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

namespace ve {

enum class QueueType : std::uint32_t { eGraphics, ePresentation, eTransfer };

struct QueueFamilyIDs {
    std::optional< std::uint32_t > graphicsFamilyID;
    std::optional< std::uint32_t > presentationFamilyID;
    std::optional< std::uint32_t > transferFamilyID;

    bool hasRequiredFamilies() const noexcept;
    static QueueFamilyIDs findQueueFamilies( const vk::PhysicalDevice device, const vk::SurfaceKHR surface );
};

} // namespace ve
