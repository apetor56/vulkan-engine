#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

namespace ve {

class DebugMessenger {
public:
    void init( const VkInstance vulkanInstance );
    void destroy( const VkInstance vulkanInstance );
    bool areValidationLayersSupported() const;
    const std::vector< const char * >& getLayers() const noexcept;
    void populateCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo ) const;

private:
    VkDebugUtilsMessengerEXT m_debugMessenger;
    const std::vector< const char * > m_validationLayers{ "VK_LAYER_KHRONOS_validation" };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                         void *pUserData );

    VkResult getDebugUtilsMessengerEXT( const VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkDebugUtilsMessengerEXT *pDebugMessenger ) const;

    void destroyDebugUtilsMessengerEXT( const VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                        const VkAllocationCallbacks *pAllocator ) const;
};

} // namespace ve