#include "swapchain.hpp"
#include "queue_family_indices.hpp"
#include "config.hpp"

#include <algorithm>
#include <numeric>
#include <array>

namespace VE {

Swapchain::Swapchain(std::shared_ptr<PhysicalDevice> physicalDevice,
                     std::shared_ptr<LogicalDevice> logicalDevice,
                     std::shared_ptr<Window> window) : m_physicalDevice { physicalDevice },
                                                       m_logicalDevice { logicalDevice},
                                                       m_window { window } {
    createSwapchain();
    createImageViews();
}

Swapchain::~Swapchain() {
    const auto logicalDeviceHandle { m_logicalDevice->getHandle() };
    vkDestroySwapchainKHR(logicalDeviceHandle, m_swapchain, nullptr);

    std::ranges::for_each(m_swapChainImageViews, [&logicalDeviceHandle](const auto& view) {
        vkDestroyImageView(logicalDeviceHandle, view, nullptr);
    });
}

void Swapchain::createSwapchain() {
    const auto physicalDeviceHandle { m_physicalDevice->getHandle() };
    const auto logicalDeviceHandle { m_logicalDevice->getHandle() };
    const auto surface { m_window->getSurface() };

    SwapchainSupportDetails swapchainSupport { querySwapChainSupport(physicalDeviceHandle, surface) };
    const VkSurfaceFormatKHR surfaceFormat { chooseSwapSurfaceFormat(swapchainSupport.formats) };
    const VkPresentModeKHR presentMode { chooseSwapPresentMode(swapchainSupport.presentModes) };
    const VkExtent2D extent { chooseSwapExtent(swapchainSupport.capabilities) };

    const uint32_t& minImageCount { swapchainSupport.capabilities.minImageCount };
    const uint32_t maxImageCount { swapchainSupport.capabilities.maxImageCount };
    uint32_t imageCount { minImageCount != maxImageCount ?  minImageCount + 1u : maxImageCount };

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1u;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices { QueueFamilyIndices::findQueueFamilies(physicalDeviceHandle, surface) };
    const std::array<uint32_t, cfg::device::queueFamiliesCount> queueFamilyIndices {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if(indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = std::size(queueFamilyIndices);
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(logicalDeviceHandle, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(logicalDeviceHandle, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDeviceHandle, m_swapchain, &imageCount, m_swapchainImages.data());
    m_format = surfaceFormat.format;
    m_extent = extent;
}

SwapchainSupportDetails Swapchain::querySwapChainSupport(const VkPhysicalDevice physicalDevice,
                                                         const VkSurfaceKHR surface) {
    SwapchainSupportDetails swapchainDetails{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainDetails.capabilities);

    uint32_t formatCount{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if(formatCount != 0u) {
        swapchainDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapchainDetails.formats.data());
    }

    uint32_t presentModeCount{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if(presentModeCount != 0u) {
        swapchainDetails.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapchainDetails.presentModes.data());
    }

    return swapchainDetails;
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
    for(const auto& availableFormat : availableFormats) {
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
    for(const auto& availablePresentMode : availablePresentModes) {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width{};
    int height{};
    glfwGetFramebufferSize(m_window->getWindowHandler(), &width, &height);

    return VkExtent2D {
        .width { std::clamp(static_cast<uint32_t> (width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width) },
        .height { std::clamp(static_cast<uint32_t> (height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height) }
    };
}

void Swapchain::createImageViews() {
    m_swapChainImageViews.resize(std::size(m_swapchainImages));
    int imageViewIndex{};

    const auto createImageView { [this, &imageViewIndex](const VkImage image) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0u;
        createInfo.subresourceRange.levelCount = 1u;
        createInfo.subresourceRange.baseArrayLayer = 0u;
        createInfo.subresourceRange.layerCount = 1u;

        if (vkCreateImageView(m_logicalDevice->getHandle(), &createInfo, nullptr, &m_swapChainImageViews.at(imageViewIndex)) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
        imageViewIndex++;
    } };

    std::ranges::for_each(m_swapchainImages, createImageView);
}

}