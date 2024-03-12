#include "debug_messenger.hpp"

#include <cstring>
#include <iostream>

namespace VE {

void DebugMessenger::init(const VkInstance vulkanInstance) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateCreateInfo(createInfo);

    if (getDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger");
    }
}

void DebugMessenger::destroy(const VkInstance vulkanInstance) {
    destroyDebugUtilsMessengerEXT(vulkanInstance, m_debugMessenger, nullptr);
}

bool DebugMessenger::areValidationLayersSupported() const {
    uint32_t layersCount{};
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

    std::vector<VkLayerProperties> allLayers(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, allLayers.data());

    size_t coveredLayers{};
    for (const auto& layerName : m_validationLayers) {
        for (const auto& layer : allLayers) {
            if (strcmp(layerName, layer.layerName) == 0) {
                coveredLayers++;
            }
        }
    }

    return coveredLayers == std::size(m_validationLayers);
}

const std::vector<const char *>& DebugMessenger::getLayers() const noexcept {
    return m_validationLayers;
}

void DebugMessenger::populateCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const {
    createInfo                 = {};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';
    return VK_FALSE;
}

VkResult DebugMessenger::getDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) const {
    auto func{ (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") };

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DebugMessenger::destroyDebugUtilsMessengerEXT(const VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                   const VkAllocationCallbacks *pAllocator) const {
    auto func{ (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT") };

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

} // namespace VE