#include "PhysicalDevice.hpp"
#include "QueueFamilyIDs.hpp"
#include "Swapchain.hpp"
#include "Config.hpp"

#include <spdlog/spdlog.h>

#include <map>
#include <algorithm>
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
    m_queueFamilies  = ve::QueueFamilyIDs::findQueueFamilies( m_physicalDevice, m_window.getSurface() );

    const auto deviceProperties{ m_physicalDevice.getProperties() };
    SPDLOG_INFO( "Picked GPU: {}", deviceProperties.deviceName.data() );
}

std::uint32_t PhysicalDevice::rate( const vk::PhysicalDevice physicalDevice ) const {
    const auto queueFamilyIndices{ ve::QueueFamilyIDs::findQueueFamilies( physicalDevice, m_window.getSurface() ) };
    const auto deviceProperties{ physicalDevice.getProperties() };
    const auto deviceFeatures{ physicalDevice.getFeatures() };

    const bool isExtensionSupportAvailable{ areRequiredExtensionsSupported( physicalDevice ) };
    bool isSwapchainAdequate{};
    if ( isExtensionSupportAvailable ) {
        const Swapchain::Details swapchainDetails{
            Swapchain::getSwapchainDetails( physicalDevice, m_window.getSurface() ) };
        isSwapchainAdequate = !swapchainDetails.formats.empty() && !swapchainDetails.presentationModes.empty();
    }

    if ( !isExtensionSupportAvailable || !isSwapchainAdequate || !deviceFeatures.geometryShader ||
         !queueFamilyIndices.hasRequiredFamilies() || !deviceFeatures.samplerAnisotropy )
        return 0U;

    std::uint32_t score{};
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

vk::PhysicalDevice PhysicalDevice::getHandler() const noexcept {
    return m_physicalDevice;
}

const ve::extentions& PhysicalDevice::getExtensions() const noexcept {
    return m_deviceExtensions;
}

[[nodiscard]] std::unordered_map< ve::FamilyType, std::uint32_t > PhysicalDevice::getQueueFamilyIDs() const noexcept {
    return m_queueFamilies.getAll();
}

} // namespace ve
