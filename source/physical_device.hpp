#pragma once

#include "vulkan_instance.hpp"
#include "window.hpp"

namespace VE {

class PhysicalDevice {
public:
    PhysicalDevice(std::shared_ptr<VulkanInstance> instance, std::shared_ptr<Window> window);

    PhysicalDevice(const PhysicalDevice& other) = delete;
    PhysicalDevice(PhysicalDevice&& other)      = delete;

    PhysicalDevice& operator=(const PhysicalDevice& other) = delete;
    PhysicalDevice& operator=(PhysicalDevice&& other)      = delete;

    VkPhysicalDevice getHandle() const;

    const std::vector<const char *> getExtensions() const noexcept;

    VkSurfaceKHR getSurface() const noexcept;

private:
    std::shared_ptr<VulkanInstance> m_vulkanInstance;
    std::shared_ptr<Window> m_window;
    VkPhysicalDevice m_physicalDevice;
    const std::vector<const char *> m_deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    void pickPhysicalDevice();

    uint32_t rate(const VkPhysicalDevice device) const;

    bool areRequiredExtensionsSupported(const VkPhysicalDevice physicalDevice) const;
};

} // namespace VE