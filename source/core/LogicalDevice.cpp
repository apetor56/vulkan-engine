#include "LogicalDevice.hpp"
#include "QueueFamilyIDs.hpp"

#include <set>
#include <stdexcept>
#include <ranges>
#include <algorithm>

namespace ve {

LogicalDevice::LogicalDevice( const ve::PhysicalDevice& physicalDevice ) : m_physicalDevice{ physicalDevice } {
    createLogicalDevice();
}

LogicalDevice::~LogicalDevice() {
    m_logicalDevice.destroy();
}

void LogicalDevice::createLogicalDevice() {
    const auto physicalDeviceHandler{ m_physicalDevice.getHandler() };
    const auto& physicalDeviceExtensions{ m_physicalDevice.getExtensions() };

    const ve::QueueFamilyIDs queueFamilyIndices{ getQueueFamilyIDs() };
    static constexpr std::uint32_t queueCount{ 1U };
    static constexpr float queuePriority{ 1.0F };

    std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos{};
    std::set< std::uint32_t > uniqueQueueFamilies{ queueFamilyIndices.graphicsFamilyID.value(),
                                                   queueFamilyIndices.presentationFamilyID.value() };

    std::ranges::for_each( uniqueQueueFamilies, [ &queueCreateInfos ]( const auto queueFamilyID ) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = vk::StructureType::eDeviceQueueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamilyID;
        queueCreateInfo.queueCount       = queueCount;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.emplace_back( queueCreateInfo );
    } );

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType                   = vk::StructureType::eDeviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos       = std::data( queueCreateInfos );
    deviceCreateInfo.queueCreateInfoCount    = static_cast< std::uint32_t >( std::size( queueCreateInfos ) );
    deviceCreateInfo.pEnabledFeatures        = nullptr;
    deviceCreateInfo.enabledExtensionCount   = static_cast< std::uint32_t >( std::size( physicalDeviceExtensions ) );
    deviceCreateInfo.ppEnabledExtensionNames = std::data( physicalDeviceExtensions );

    m_logicalDevice = physicalDeviceHandler.createDevice( deviceCreateInfo, nullptr );

    constexpr std::uint32_t queueIndex{ 0U };
    m_graphicsQueue     = m_logicalDevice.getQueue( queueFamilyIndices.graphicsFamilyID.value(), queueIndex );
    m_presentationQueue = m_logicalDevice.getQueue( queueFamilyIndices.graphicsFamilyID.value(), queueIndex );
}

vk::Device LogicalDevice::getHandler() const noexcept {
    return m_logicalDevice;
}

vk::Queue LogicalDevice::getGraphicsQueue() const noexcept {
    return m_graphicsQueue;
}

vk::Queue LogicalDevice::getPresentationQueue() const noexcept {
    return m_presentationQueue;
}

[[nodiscard]] ve::QueueFamilyIDs LogicalDevice::getQueueFamilyIDs() const noexcept {
    return m_physicalDevice.getQueueFamilyIDs();
}

} // namespace ve
