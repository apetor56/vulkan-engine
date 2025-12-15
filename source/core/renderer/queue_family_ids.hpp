#pragma once

#include <vulkan/vulkan.hpp>

#include <unordered_map>

namespace ve {

enum class QueueType : uint32_t { eGraphics, ePresentation, eTransfer };
enum class FamilyType : uint32_t { eGraphics, ePresentation, eTransfer };

using QueueFamilyMap = std::unordered_map< ve::FamilyType, uint32_t >;

struct QueueFamilyIDs {
    bool hasRequiredFamilies() const noexcept;
    static QueueFamilyIDs findQueueFamilies( const vk::PhysicalDevice device, const vk::SurfaceKHR surface );
    std::unordered_map< FamilyType, uint32_t > getAll() const noexcept { return m_familyIndices; }

private:
    std::unordered_map< FamilyType, uint32_t > m_familyIndices;

    void add( FamilyType type, uint32_t familyID );
};

} // namespace ve
