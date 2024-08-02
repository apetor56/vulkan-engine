#include "PhysicalDevice.hpp"
#include "QueueFamilyIndices.hpp"
#include "Swapchain.hpp"
#include "Config.hpp"

#include <spdlog/spdlog.h>

#include <map>
#include <algorithm>
#include <cstring>
#include <ranges>

namespace ve {

PhysicalDevice::PhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window )
    : m_instance{ instance }, m_window{ window } {
    pickPhysicalDevice();
}

void PhysicalDevice::pickPhysicalDevice() {
    const auto devices{ m_instance.get().enumeratePhysicalDevices() };
    if ( std::size( devices ) == 0U )
        throw std::runtime_error( "failed to find GPU with Vulkan support" );

    std::map< std::uint32_t, vk::PhysicalDevice > deviceCandidates{};
    std::ranges::for_each( devices, [ this, &deviceCandidates ]( const auto& device ) {
        deviceCandidates.insert( { rate( device ), device } );
    } );

    const auto [ bestRate, bestDevice ]{ *std::rbegin( deviceCandidates ) };
    if ( bestRate == 0U )
        throw std::runtime_error( "failed to find suitable device" );

    m_physicalDevice = bestDevice;
    m_queueFamilies  = ve::QueueFamilyIndices::findQueueFamilies( m_physicalDevice, m_window.getSurface() );

    const auto deviceProperties{ m_physicalDevice.getProperties() };
    SPDLOG_INFO( "Picked GPU: {}", deviceProperties.deviceName.data() );
}

std::uint32_t PhysicalDevice::rate( const vk::PhysicalDevice physicalDevice ) const {
    const auto queueFamilyIndices{ ve::QueueFamilyIndices::findQueueFamilies( physicalDevice, m_window.getSurface() ) };
    const auto deviceProperties{ physicalDevice.getProperties() };
    const auto deviceFeatures{ physicalDevice.getFeatures() };

    const bool isExtensionSupportAvailable{ areRequiredExtensionsSupported( physicalDevice ) };
    bool isSwapchainAdequate{};
    if ( isExtensionSupportAvailable ) {
        SwapchainSupportDetails swapchainSupport{
            Swapchain::querySwapChainSupport( physicalDevice, m_window.getSurface() ) };
        isSwapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.formats.empty();
    }

    if ( !isExtensionSupportAvailable || !isSwapchainAdequate || !deviceFeatures.geometryShader ||
         !queueFamilyIndices.hasRequiredFamilies() ) {
        return 0U;
    }

    std::uint32_t score{};
    if ( deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu )
        score += cfg::gpu::discreteGpuValue;

    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

bool PhysicalDevice::areRequiredExtensionsSupported( const VkPhysicalDevice physicalDevice ) const {
    uint32_t extensionCount{};
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionCount, nullptr );

    std::vector< VkExtensionProperties > allExtensions( extensionCount );
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionCount, allExtensions.data() );

    size_t coveredExtensions{};
    for ( const char *requiredExtensionName : m_deviceExtensions ) {
        for ( const VkExtensionProperties& extension : allExtensions ) {
            if ( strcmp( extension.extensionName, requiredExtensionName ) == 0 ) {
                coveredExtensions++;
            }
        }
    }

    return coveredExtensions == std::size( m_deviceExtensions );
}

vk::PhysicalDevice PhysicalDevice::getHandler() const noexcept {
    return m_physicalDevice;
}

const ve::extentions& PhysicalDevice::getExtensions() const noexcept {
    return m_deviceExtensions;
}

ve::QueueFamilyIndices PhysicalDevice::getQueueFamilies() const noexcept {
    return m_queueFamilies;
}

} // namespace ve
