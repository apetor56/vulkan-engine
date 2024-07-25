#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <span>

namespace ve {

using layers = std::vector< const char * >;

class DebugMessenger {
public:
    DebugMessenger( VkInstance instance );
    void destroy();

    static bool areValidationLayersSupported();
    static std::span< const char * > getLayers() noexcept;
    VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsCreateInfo() const noexcept;

private:
    VkDebugUtilsMessengerEXT m_debugMessenger{};
    inline static layers m_validationLayers{ "VK_LAYER_KHRONOS_validation" };
    VkInstance m_vulkanInstance{};

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                         void *pUserData );

    VkResult createDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkDebugUtilsMessengerEXT *pDebugMessenger ) const;

    void destroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                        const VkAllocationCallbacks *pAllocator ) const;
};

} // namespace ve
