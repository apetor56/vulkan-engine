#pragma once

#include "vulkan_instance.hpp"
#include "window.hpp"
#include <vector>
#include <optional>
#include <memory>

namespace VE {

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Device {
public:
    explicit Device(std::shared_ptr<VulkanInstance> vulkanInstance,
                    std::shared_ptr<Window> window);

    ~Device();

    Device(const Device& other) = delete;
    Device& operator=(const Device& other) = delete;

    VkDevice getDevice() const;

private:
    std::shared_ptr<VulkanInstance> m_vulkanInstance;
    const std::vector<const char*> m_deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    std::shared_ptr<Window> m_window;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_format;
    VkExtent2D m_extent;

    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();

    uint32_t deviceRate(const VkPhysicalDevice device) const;

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const;

    bool areRequiredExtensionsSupported(const VkPhysicalDevice physicalDevice) const;

    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice) const;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
};

}