#pragma once

#include "QueueFamilyIDs.hpp"

#include "utils/NonCopyable.hpp"
#include "utils/NonMovable.hpp"

namespace ve {

class VulkanInstance;
class Window;
class LogicalDevice;

class PhysicalDevice : public utils::NonCopyable,
                       public utils::NonMovable {
public:
    PhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window );

    vk::PhysicalDevice get() const noexcept { return m_physicalDevice; }
    const std::vector< const char * >& getExtensions() const noexcept { return m_deviceExtensions; }
    [[nodiscard]] ve::QueueFamilyMap getQueueFamilyIDs() const noexcept { return m_queueFamilies.getAll(); }
    vk::SampleCountFlagBits getMaxSamplesCount() const noexcept;

private:
    ve::QueueFamilyIDs m_queueFamilies{};
    const std::vector< const char * > m_deviceExtensions{ vk::KHRSwapchainExtensionName,
                                                          vk::KHRDynamicRenderingExtensionName };
    vk::PhysicalDevice m_physicalDevice{};

    void pickPhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window );
    uint32_t rate( const vk::PhysicalDevice device, const VkSurfaceKHR surface ) const noexcept;
    bool areRequiredExtensionsSupported( const vk::PhysicalDevice physicalDevice ) const;
};

} // namespace ve
