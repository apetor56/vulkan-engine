#pragma once

#include <vulkan/vulkan.hpp>

#include <unordered_map>
#include <cstdint>

namespace ve {

enum class QueueType : std::uint32_t { eGraphics, ePresentation, eTransfer };
enum class FamilyType : std::uint32_t { eGraphics, ePresentation, eTransfer };

struct QueueFamilyIDs {
    bool hasRequiredFamilies() const noexcept;
    static QueueFamilyIDs findQueueFamilies( const vk::PhysicalDevice device, const vk::SurfaceKHR surface );
    std::unordered_map< FamilyType, std::uint32_t > getAll() const;

private:
    std::unordered_map< FamilyType, std::uint32_t > m_familyIndices;

    void add( FamilyType type, std::uint32_t familyID );
};

} // namespace ve
