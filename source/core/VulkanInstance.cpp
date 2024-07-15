#include "VulkanInstance.hpp"

#include <iostream>

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

    const auto extensions{ getRequiredInstanceExtensions() };

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

ve::extentions VulkanInstance::getRequiredInstanceExtensions() {
    std::uint32_t extensionCount{};
    const char **extensions{ glfwGetRequiredInstanceExtensions( &extensionCount ) };
    ve::extentions instanceExtensions{ extensions, extensions + extensionCount };

    return instanceExtensions;
}

vk::Instance VulkanInstance::get() const noexcept {
    return m_instance;
}

void VulkanInstance::showAllSupportedExtensions() {
    std::uint32_t extensionCount{};
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

    std::vector< VkExtensionProperties > extensions( extensionCount );
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );

    std::cout << "all available vulkan instance extensions: \n";
    std::ranges::for_each( extensions,
                           []( const auto extension ) { std::cout << '\t' << extension.extensionName << '\n'; } );
}

} // namespace ve
