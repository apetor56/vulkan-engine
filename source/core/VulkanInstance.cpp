#include "VulkanInstance.hpp"

#include <iostream>
#include <ranges>

namespace ve {

VulkanInstance::VulkanInstance() {
    glfwInit();
    createVulkanInstance();
    showAllSupportedExtensions();
}

VulkanInstance::~VulkanInstance() {
    vkDestroyInstance( m_instance, nullptr );
    glfwTerminate();
}

void VulkanInstance::createVulkanInstance() {
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
    instanceInfo.enabledExtensionCount   = static_cast< std::uint32_t >( extensions.size() );
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount       = 0U;
    instanceInfo.pNext                   = nullptr;

    if ( vk::createInstance( &instanceInfo, nullptr, &m_instance ) != vk::Result::eSuccess )
        throw std::runtime_error( "failed to create vulkan instance" );
}

ve::extentions VulkanInstance::getRequiredInstanceExtensions() const {
    std::uint32_t extensionCount{};
    const char **extensions{ glfwGetRequiredInstanceExtensions( &extensionCount ) };
    ve::extentions instanceExtensions{ extensions, extensions + extensionCount };

    return instanceExtensions;
}

vk::Instance VulkanInstance::get() const noexcept {
    return m_instance;
}

void VulkanInstance::showAllSupportedExtensions() const {
    std::uint32_t extensionCount{};
    vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

    std::vector< vk::ExtensionProperties > extensions( extensionCount );
    vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );

    const auto getName{ []( const auto& extension ) { return extension.extensionName; } };
    const auto extensionNames{ extensions | std::views::transform( getName ) };

    std::cout << "all available vulkan instance extensions: \n";
    std::ranges::for_each( extensionNames, []( const auto& extension ) { std::cout << '\t' << extension << '\n'; } );
}

} // namespace ve
