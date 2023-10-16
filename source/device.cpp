#include "device.hpp"
#include "window.hpp"
#include "config.hpp"
#include <stdexcept>
#include <cstring>
#include <map>
#include <iostream>
#include <algorithm>
#include <set>
#include <ranges>
#include <limits>
#include <array>

namespace VE {

Device::Device(std::shared_ptr<Window> window) : m_physicalDevice(VK_NULL_HANDLE), m_window(window) {
    createVulkanInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
}

Device::~Device() {
    if (m_isValidationEnabled) {
        destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

VkDevice Device::getDevice() const {
    return m_device;
}

void Device::createVulkanInstance() {
    if(m_isValidationEnabled && !checkValidationLayersSupport()) {
        throw std::runtime_error("validation layers requested but not available");
    }

    #ifndef NDEBUG
        showAllSupportedExtensions();
    #endif

    // optional
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext              = nullptr;
    appInfo.pApplicationName   = "Vulkan learning";
    appInfo.pEngineName        = nullptr;
    appInfo.apiVersion         = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

    const auto extensions { getRequiredInstanceExtensions() };

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if(m_isValidationEnabled) {
        instanceInfo.enabledLayerCount   = static_cast<uint32_t>(std::size(m_validationLayers));
        instanceInfo.ppEnabledLayerNames = m_validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        instanceInfo.enabledLayerCount = 0u;
        instanceInfo.pNext             = nullptr;
    }

    if(vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vulkan instance");
    }
}

void Device::setupDebugMessenger() {
    if (!m_isValidationEnabled) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (getDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger");
    }
}

std::vector<const char*> Device::getRequiredInstanceExtensions() const {
    uint32_t glfwExtensionCount{};
    const char **glfwExtensions { glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(m_isValidationEnabled) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Device::showAllSupportedExtensions() const {
    uint32_t extensionCount{};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "all available vulkan instance extensions: \n";
    for(const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

bool Device::checkValidationLayersSupport() const {
    uint32_t layersCount{};
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

    std::vector<VkLayerProperties> allLayers(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, allLayers.data());

    size_t coveredLayers{};
    for(const auto& layerName : m_validationLayers) {
        for(const auto& layer : allLayers) {
            if(strcmp(layerName, layer.layerName) == 0) {
                coveredLayers++;
            }
        }
    }

    return coveredLayers == std::size(m_validationLayers);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Device::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                     void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';
    return VK_FALSE;
}

VkResult Device::getDebugUtilsMessengerEXT(VkInstance instance, 
                                           const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator,
                                           VkDebugUtilsMessengerEXT* pDebugMessenger) const {
    auto func { (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") };

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Device::destroyDebugUtilsMessengerEXT(VkInstance instance,
                                           VkDebugUtilsMessengerEXT debugMessenger,
                                           const VkAllocationCallbacks* pAllocator) const {
    auto func { (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT") };

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const {
    createInfo                 = {};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void Device::pickPhysicalDevice() {
    uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if(deviceCount == 0u) {
        throw std::runtime_error("failed to find GPU with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    std::multimap<uint32_t, VkPhysicalDevice> candidates{};
    
    const auto makeDeviceRatePair { [this, &candidates](const auto& device) {
        candidates.insert(std::make_pair(deviceRate(device), device));
    } };
    std::ranges::for_each(devices, makeDeviceRatePair);

    if(std::rbegin(candidates)->first > 0u) {
        m_physicalDevice = std::rbegin(candidates)->second;

        #ifndef NDEBUG
            VkPhysicalDeviceProperties deviceProperties{};
            vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
            std::cout << "picked GPU: " << deviceProperties.deviceName << '\n';
        #endif
    }
    else {
        throw std::runtime_error("failed to find suitable device");
    }
}

uint32_t Device::deviceRate(const VkPhysicalDevice physicalDevice) const {
    QueueFamilyIndices queueIndices { findQueueFamilies(physicalDevice) };
    VkPhysicalDeviceProperties deviceProperties{};
    VkPhysicalDeviceFeatures deviceFeatures{};

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    const bool isExtensionSupportAvailable { areRequiredExtensionsSupported(physicalDevice) };
    bool isSwapchainAdequate {};
    if(isExtensionSupportAvailable) {
        SwapchainSupportDetails swapchainSupport { querySwapChainSupport(physicalDevice) };
        isSwapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.formats.empty();
    }

    if(!isExtensionSupportAvailable || !isSwapchainAdequate || !deviceFeatures.geometryShader || !queueIndices.isComplete()) {
        return 0u;
    }

    uint32_t score{};
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += cfg::gpu::discreteGpuValue;
    }
    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

QueueFamilyIndices Device::findQueueFamilies(const VkPhysicalDevice device) const {
    QueueFamilyIndices queueFamilyIndices{};

    uint32_t queueFamilyCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t queueFamilyIndex{};
    VkBool32 isPresentSupportAvailable{ false };

    for(const auto& queueFamily : queueFamilies) {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyIndices.graphicsFamily = queueFamilyIndex;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, m_surface, &isPresentSupportAvailable);

        if(isPresentSupportAvailable) {
            queueFamilyIndices.presentFamily = queueFamilyIndex;
        }

        if(queueFamilyIndices.isComplete()) {
            break;
        }

        queueFamilyIndex++;
    }

    return queueFamilyIndices;
}

void Device::createLogicalDevice() {
    QueueFamilyIndices queueFamilyIndices { findQueueFamilies(m_physicalDevice) };
    constexpr uint32_t queueCount{ 1u };
    constexpr float queuePriority { 1.f };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies {
        queueFamilyIndices.graphicsFamily.value(),
        queueFamilyIndices.presentFamily.value()
    };

    for(const auto& queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = queueCount;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};      // empty for now

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t> (std::size(queueCreateInfos));
    deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t> (std::size(m_deviceExtensions));
    deviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    if(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device");
    }

    constexpr uint32_t queueIndex{};
    vkGetDeviceQueue(m_device, queueFamilyIndices.graphicsFamily.value(), queueIndex, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, queueFamilyIndices.presentFamily.value(), queueIndex, &m_presentQueue);
}

void Device::createSurface() {
    if(glfwCreateWindowSurface(m_instance, m_window->getWindowHandler(), nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}

bool Device::areRequiredExtensionsSupported(const VkPhysicalDevice physicalDevice) const {
    uint32_t extensionCount{};
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> allExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, allExtensions.data());

    // const auto getExtensionName { [](const VkExtensionProperties& extension) {
    //     return extension.extensionName;
    // }};
    // const auto& allExtensionsNames { allExtensions | std::views::transform(getExtensionName) };

    // const auto isRequiredExtensionFound { [&allExtensionsNames](const char *extensionName) {
    //     return std::ranges::find(allExtensionsNames, extensionName) != std::end(allExtensionsNames);
    // } };
    // return std::ranges::all_of(m_deviceExtensions, isRequiredExtensionFound);
    size_t coveredExtensions {};
    for(const char *requiredExtensionName : m_deviceExtensions) {
        for(const VkExtensionProperties& extension : allExtensions) {
            if(strcmp(extension.extensionName, requiredExtensionName) == 0) {
                coveredExtensions++;
            }
        }
    }

    return coveredExtensions == std::size(m_deviceExtensions);
}

SwapchainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice physicalDevice) const {
    SwapchainSupportDetails swapchainDetails{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &swapchainDetails.capabilities);

    uint32_t formatCount{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, nullptr);
    if(formatCount != 0u) {
        swapchainDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, swapchainDetails.formats.data());
    }

    uint32_t presentModeCount{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);
    if(presentModeCount != 0u) {
        swapchainDetails.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, swapchainDetails.presentModes.data());
    }

    return swapchainDetails;
}

VkSurfaceFormatKHR Device::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
    for(const auto& availableFormat : availableFormats) {
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Device::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
    for(const auto& availablePresentMode : availablePresentModes) {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Device::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
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

void Device::createSwapchain() {
    SwapchainSupportDetails swapchainSupport { querySwapChainSupport(m_physicalDevice) };
    const VkSurfaceFormatKHR surfaceFormat { chooseSwapSurfaceFormat(swapchainSupport.formats) };
    const VkPresentModeKHR presentMode { chooseSwapPresentMode(swapchainSupport.presentModes) };
    const VkExtent2D extent { chooseSwapExtent(swapchainSupport.capabilities) };

    /* uint32_t imageCount { swapchainSupport.capabilities.minImageCount + 1u };
    // if capabilities.maxImageCount == 0 then there is no maximum limit
    if(swapchainSupport.capabilities.maxImageCount > 0u && imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    } */
    const uint32_t& minImageCount { swapchainSupport.capabilities.minImageCount };
    const uint32_t maxImageCount { swapchainSupport.capabilities.maxImageCount };
    uint32_t imageCount { minImageCount != maxImageCount ?  minImageCount + 1u : maxImageCount };

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1u;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices { findQueueFamilies(m_physicalDevice) };
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

    if(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
    m_format = surfaceFormat.format;
    m_extent = extent;
}

}