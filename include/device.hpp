#pragma once

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
    explicit Device(std::shared_ptr<Window> window);
    ~Device();

    Device(const Device& other) = delete;
    Device& operator=(const Device& other) = delete;

    VkDevice getDevice() const;

private:
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    const std::vector<const char*> m_validationLayers { "VK_LAYER_KHRONOS_validation" };
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

    #ifdef NDEBUG
        const bool m_isValidationEnabled { false };
    #else
        const bool m_isValidationEnabled { true };
    #endif

    void createVulkanInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();

    std::vector<const char*> getRequiredInstanceExtensions() const;

    void showAllSupportedExtensions() const;

    bool checkValidationLayersSupport() const;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);

    VkResult getDebugUtilsMessengerEXT(VkInstance instance, 
                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator,
                                       VkDebugUtilsMessengerEXT* pDebugMessenger) const;

    void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator) const;

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;


    uint32_t deviceRate(const VkPhysicalDevice device) const;

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const;

    bool areRequiredExtensionsSupported(const VkPhysicalDevice physicalDevice) const;

    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice) const;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
};

}