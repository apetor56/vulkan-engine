#include "DebugMessenger.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <ranges>
#include <algorithm>
#include <cstdint>

namespace ve {

DebugMessenger::DebugMessenger( VkInstance instance ) : m_vulkanInstance{ instance } {
    const auto createInfo{ getDebugUtilsCreateInfo() };
    if ( createDebugUtilsMessengerEXT( m_vulkanInstance, &createInfo, nullptr, &m_debugMessenger ) != VK_SUCCESS )
        throw std::runtime_error( "failed to set up debug messenger" );
}

void DebugMessenger::destroy() {
    destroyDebugUtilsMessengerEXT( m_vulkanInstance, m_debugMessenger, nullptr );
}

bool DebugMessenger::areValidationLayersSupported() {
    std::uint32_t layersCount{};
    vkEnumerateInstanceLayerProperties( &layersCount, nullptr );

    std::vector< VkLayerProperties > allAvailableLayers( layersCount );
    vkEnumerateInstanceLayerProperties( &layersCount, std::data( allAvailableLayers ) );

    const auto getName{ []( const auto& layer ) { return std::string_view{ layer.layerName }; } };
    const auto allAvailableLayerNames{ allAvailableLayers | std::views::transform( getName ) };
    const auto isLayerSupported{ [ &allAvailableLayerNames ]( const auto& layerName ) {
        return std::ranges::contains( allAvailableLayerNames, layerName );
    } };

    return std::ranges::all_of( m_validationLayers, isLayerSupported );
}

std::span< const char * > DebugMessenger::getLayers() noexcept {
    return m_validationLayers;
}

VkDebugUtilsMessengerCreateInfoEXT DebugMessenger::getDebugUtilsCreateInfo() const noexcept {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    return createInfo;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT,
                                                              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                              void * ) {
    if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
        SPDLOG_ERROR( "\nmessage name: {}\nmessage: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage );
    else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
        SPDLOG_WARN( "\nmessage name: {}\nmessage: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage );
    return VK_FALSE;
}

VkResult DebugMessenger::createDebugUtilsMessengerEXT( VkInstance instance,
                                                       const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       VkDebugUtilsMessengerEXT *pDebugMessenger ) const {
    auto func{
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" ) };

    if ( func != nullptr )
        return func( instance, pCreateInfo, pAllocator, pDebugMessenger );

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DebugMessenger::destroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                    const VkAllocationCallbacks *pAllocator ) const {
    auto func{
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" ) };

    if ( func != nullptr )
        func( instance, debugMessenger, pAllocator );
}

} // namespace ve
