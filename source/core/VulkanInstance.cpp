#include "VulkanInstance.hpp"
#include "DebugMessenger.hpp"
#include "Config.hpp"

#include <iostream>
#include <ranges>

namespace ve {

VulkanInstance::VulkanInstance() {
    glfwInit();
    createVulkanInstance();

    if ( cfg::debug::areValidationLayersEnabled )
        m_debugMessenger.emplace( m_instance );

    showAllSupportedExtensions();
}

VulkanInstance::~VulkanInstance() {
    if ( m_debugMessenger.has_value() )
        m_debugMessenger->destroy();

    vkDestroyInstance( m_instance, nullptr );
    glfwTerminate();
}

void VulkanInstance::createVulkanInstance() {
    if ( cfg::debug::areValidationLayersEnabled && !DebugMessenger::areValidationLayersSupported() )
        throw std::runtime_error( "validation layer requested, but not available" );

    vk::ApplicationInfo appInfo{};
    appInfo.sType              = vk::StructureType::eApplicationInfo;
    appInfo.pNext              = nullptr;
    appInfo.pApplicationName   = "vulkan engine";
    appInfo.pEngineName        = nullptr;
    appInfo.apiVersion         = vk::ApiVersion10;
    appInfo.applicationVersion = vk::makeApiVersion( 0, 1, 0, 0 );
    appInfo.engineVersion      = vk::makeApiVersion( 0, 1, 0, 0 );

    auto extensions{ getRequiredInstanceExtensions() };

    vk::InstanceCreateInfo instanceInfo{};
    instanceInfo.sType                   = vk::StructureType::eInstanceCreateInfo;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledExtensionCount   = static_cast< std::uint32_t >( std::size( extensions ) );
    instanceInfo.ppEnabledExtensionNames = std::data( extensions );
    instanceInfo.pNext                   = nullptr;

    if ( cfg::debug::areValidationLayersEnabled ) {
        const auto validationLayers{ DebugMessenger::getLayers() };
        instanceInfo.enabledLayerCount   = static_cast< std::uint32_t >( std::size( validationLayers ) );
        instanceInfo.ppEnabledLayerNames = std::data( validationLayers );
    } else {
        instanceInfo.enabledLayerCount   = 0U;
        instanceInfo.ppEnabledLayerNames = nullptr;
    }

    if ( vk::createInstance( &instanceInfo, nullptr, &m_instance ) != vk::Result::eSuccess )
        throw std::runtime_error( "failed to create vulkan instance" );
}

ve::extentions VulkanInstance::getRequiredInstanceExtensions() const {
    std::uint32_t extensionCount{};
    const char **extensions{ glfwGetRequiredInstanceExtensions( &extensionCount ) };
    ve::extentions instanceExtensions{ extensions, extensions + extensionCount };

    if ( cfg::debug::areValidationLayersEnabled )
        instanceExtensions.emplace_back( vk::EXTDebugUtilsExtensionName );

    return instanceExtensions;
}

vk::Instance VulkanInstance::get() const noexcept {
    return m_instance;
}

void VulkanInstance::showAllSupportedExtensions() const {
    std::uint32_t extensionCount{};
    vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

    std::vector< vk::ExtensionProperties > extensions( extensionCount );
    vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, std::data( extensions ) );

    const auto getName{ []( const auto& extension ) { return extension.extensionName; } };
    const auto extensionNames{ extensions | std::ranges::views::transform( getName ) };

    std::cout << "all available vulkan instance extensions: \n";
    std::ranges::for_each( extensionNames, []( const auto& extension ) { std::cout << '\t' << extension << '\n'; } );
}

} // namespace ve
