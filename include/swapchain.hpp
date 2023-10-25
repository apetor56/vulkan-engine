#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "logical_device.hpp"

namespace VE {

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain {
public:
    Swapchain(std::shared_ptr<PhysicalDevice> physicalDevice,
              std::shared_ptr<LogicalDevice> logicalDevice,
              std::shared_ptr<Window> window);

    ~Swapchain();

    Swapchain(const Swapchain& other) = delete;
    Swapchain(Swapchain&& other) = delete;

    Swapchain& operator=(const Swapchain& other) = delete;
    Swapchain& operator=(Swapchain&& other) = delete;

    static SwapchainSupportDetails querySwapChainSupport(const VkPhysicalDevice physicalDevice,
                                                         const VkSurfaceKHR surface);

private:
    std::shared_ptr<PhysicalDevice> m_physicalDevice;
    std::shared_ptr<LogicalDevice> m_logicalDevice;
    std::shared_ptr<Window> m_window;

    VkSwapchainKHR m_swapchain;
    VkFormat m_format;
    VkExtent2D m_extent;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    void createSwapchain();
    void createImageViews();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
};

}