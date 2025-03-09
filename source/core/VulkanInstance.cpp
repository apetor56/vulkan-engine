#include "VulkanInstance.hpp"
#include "DebugMessenger.hpp"
#include "Config.hpp"
#include "utils/Common.hpp"

#include <GLFW/glfw3.h>

#include <ranges>

namespace ve {

VulkanInstance::VulkanInstance() {
    glfwInit();
    createVulkanInstance();

    if ( cfg::debug::areValidationLayersEnabled )
        m_debugMessenger.emplace( m_instance );
}

VulkanInstance::~VulkanInstance() {
    if ( m_debugMessenger.has_value() )
        m_debugMessenger->destroy();

    m_instance.destroy();
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
    instanceInfo.enabledExtensionCount   = utils::size( extensions );
    instanceInfo.ppEnabledExtensionNames = std::data( extensions );
    instanceInfo.pNext                   = nullptr;

    if ( cfg::debug::areValidationLayersEnabled ) {
        const auto validationLayers{ DebugMessenger::getLayers() };
        instanceInfo.enabledLayerCount   = utils::size( validationLayers );
        instanceInfo.ppEnabledLayerNames = std::data( validationLayers );
    } else {
        instanceInfo.enabledLayerCount   = 0U;
        instanceInfo.ppEnabledLayerNames = nullptr;
    }

    vk::detail::resultCheck( vk::createInstance( &instanceInfo, nullptr, &m_instance ),
                             "failed to create vulkan instance" );
}

std::vector< const char * > VulkanInstance::getRequiredInstanceExtensions() const {
    uint32_t extensionCount{};
    const char **extensions{ glfwGetRequiredInstanceExtensions( &extensionCount ) };
    std::vector< const char * > instanceExtensions{ extensions, extensions + extensionCount };

    if constexpr ( cfg::debug::areValidationLayersEnabled )
        instanceExtensions.emplace_back( vk::EXTDebugUtilsExtensionName );

    return instanceExtensions;
}

} // namespace ve
