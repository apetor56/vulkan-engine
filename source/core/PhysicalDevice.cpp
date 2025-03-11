#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "Swapchain.hpp"
#include "Config.hpp"

#include <spdlog/spdlog.h>

#include <map>
#include <algorithm>
#include <ranges>

namespace ve {

PhysicalDevice::PhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window ) {
    pickPhysicalDevice( instance, window );
}

void PhysicalDevice::pickPhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window ) {
    const auto devices{ instance.get().enumeratePhysicalDevices() };
    if ( std::size( devices ) == 0U )
        throw std::runtime_error( "failed to find GPU with Vulkan support" );

    std::map< uint32_t, vk::PhysicalDevice > deviceCandidates{};
    std::ranges::for_each( devices, [ this, &window, &deviceCandidates ]( const auto device ) {
        deviceCandidates.insert( { rate( device, window.getSurface() ), device } );
    } );

    const auto [ bestRate, bestDevice ]{ *std::rbegin( deviceCandidates ) };
    if ( bestRate == 0U )
        throw std::runtime_error( "failed to find suitable device" );

    m_physicalDevice = bestDevice;
    m_queueFamilies  = ve::QueueFamilyIDs::findQueueFamilies( m_physicalDevice, window.getSurface() );

    const auto deviceProperties{ m_physicalDevice.getProperties() };
    SPDLOG_INFO( "Picked GPU: {}", deviceProperties.deviceName.data() );
}

uint32_t PhysicalDevice::rate( const vk::PhysicalDevice physicalDevice, const VkSurfaceKHR surface ) const noexcept {
    const auto queueFamilyIndices{ ve::QueueFamilyIDs::findQueueFamilies( physicalDevice, surface ) };
    const auto deviceProperties{ physicalDevice.getProperties() };
    const auto deviceFeatures{ physicalDevice.getFeatures() };

    const bool isExtensionSupportAvailable{ areRequiredExtensionsSupported( physicalDevice ) };
    bool isSwapchainAdequate{};
    if ( isExtensionSupportAvailable ) {
        const Swapchain::Details swapchainDetails{ Swapchain::getSwapchainDetails( physicalDevice, surface ) };
        isSwapchainAdequate = !swapchainDetails.formats.empty() && !swapchainDetails.presentationModes.empty();
    }

    vk::PhysicalDeviceVulkan12Features supportedFeaturesV12{};
    vk::PhysicalDeviceFeatures2 features2{};
    features2.pNext = &supportedFeaturesV12;
    physicalDevice.getFeatures2( &features2 );

    if ( !isExtensionSupportAvailable || !isSwapchainAdequate || !deviceFeatures.geometryShader ||
         !queueFamilyIndices.hasRequiredFamilies() || !deviceFeatures.samplerAnisotropy ||
         !supportedFeaturesV12.bufferDeviceAddress )
        return 0U;

    uint32_t score{};
    if ( deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu )
        score += cfg::gpu::discreteGpuValue;

    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

bool PhysicalDevice::areRequiredExtensionsSupported( const vk::PhysicalDevice physicalDevice ) const {
    const auto allDeviceExtensions{ physicalDevice.enumerateDeviceExtensionProperties() };
    const auto getName{
        []( const auto& extensionProperty ) { return std::string_view( extensionProperty.extensionName ); } };
    const auto allDeviceExtensionsNames{ allDeviceExtensions | std::views::transform( getName ) };

    const auto isRequiredExtensionSupported{ [ &allDeviceExtensionsNames ]( const auto& requiredExtensionName ) {
        return std::ranges::find( allDeviceExtensionsNames, requiredExtensionName ) !=
               std::end( allDeviceExtensionsNames );
    } };

    return std::ranges::all_of( m_deviceExtensions, isRequiredExtensionSupported );
}

} // namespace ve
