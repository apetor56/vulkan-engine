#include "PhysicalDevice.hpp"
#include "QueueFamilyIndices.hpp"
#include "Swapchain.hpp"
#include "Config.hpp"

#include <map>
#include <algorithm>
#include <cstring>
#include <ranges>

#ifndef NDEBUG
#include <iostream>
#endif

namespace ve {

PhysicalDevice::PhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window )
    : m_instance{ instance }, m_window{ window } {
    pickPhysicalDevice();
}

void PhysicalDevice::pickPhysicalDevice() {
    uint32_t deviceCount{};
    vkEnumeratePhysicalDevices( m_instance.get(), &deviceCount, nullptr );

    if ( deviceCount == 0u ) {
        throw std::runtime_error( "failed to find GPU with Vulkan support" );
    }

    std::vector< VkPhysicalDevice > devices( deviceCount );
    vkEnumeratePhysicalDevices( m_instance.get(), &deviceCount, devices.data() );

    std::multimap< uint32_t, VkPhysicalDevice > candidates{};
    const auto makeRateDevicePair{ [ this, &candidates ]( const auto& device ) {
        candidates.insert( std::make_pair( rate( device ), device ) );
    } };
    std::ranges::for_each( devices, makeRateDevicePair );

    const auto& bestDeviceRate{ std::rbegin( candidates )->first };
    if ( bestDeviceRate > 0u ) {
        m_physicalDevice = std::rbegin( candidates )->second;

#ifndef NDEBUG
        VkPhysicalDeviceProperties deviceProperties{};
        vkGetPhysicalDeviceProperties( m_physicalDevice, &deviceProperties );
        std::cout << "picked GPU: " << deviceProperties.deviceName << '\n';
#endif
    } else {
        throw std::runtime_error( "failed to find suitable device" );
    }
}

uint32_t PhysicalDevice::rate( const VkPhysicalDevice physicalDevice ) const {
    QueueFamilyIndices queueIndices{ QueueFamilyIndices::findQueueFamilies( physicalDevice, m_window.getSurface() ) };
    VkPhysicalDeviceProperties deviceProperties{};
    VkPhysicalDeviceFeatures deviceFeatures{};

    vkGetPhysicalDeviceProperties( physicalDevice, &deviceProperties );
    vkGetPhysicalDeviceFeatures( physicalDevice, &deviceFeatures );

    const bool isExtensionSupportAvailable{ areRequiredExtensionsSupported( physicalDevice ) };
    bool isSwapchainAdequate{};
    if ( isExtensionSupportAvailable ) {
        SwapchainSupportDetails swapchainSupport{
            Swapchain::querySwapChainSupport( physicalDevice, m_window.getSurface() ) };
        isSwapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.formats.empty();
    }

    if ( !isExtensionSupportAvailable || !isSwapchainAdequate || !deviceFeatures.geometryShader ||
         !queueIndices.isComplete() ) {
        return 0u;
    }

    uint32_t score{};
    if ( deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
        score += cfg::gpu::discreteGpuValue;
    }
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

VkPhysicalDevice PhysicalDevice::getHandler() const {
    return m_physicalDevice;
}

const extentions& PhysicalDevice::getExtensions() const noexcept {
    return m_deviceExtensions;
}

} // namespace ve
