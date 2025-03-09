#include "LogicalDevice.hpp"
#include "QueueFamilyIDs.hpp"
#include "utils/Common.hpp"

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
    const auto& physicalDeviceExtensions{ m_physicalDevice.getExtensions() };
    const auto queueFamilyIndices{ m_physicalDevice.getQueueFamilyIDs() };
    static constexpr uint32_t queueCount{ 1U };
    static constexpr float queuePriority{ 1.0F };

    std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos{};
    std::set< uint32_t > uniqueQueueFamilies{ queueFamilyIndices.at( FamilyType::eGraphics ),
                                              queueFamilyIndices.at( FamilyType::ePresentation ),
                                              queueFamilyIndices.at( FamilyType::eTransfer ) };

    std::ranges::for_each( uniqueQueueFamilies, [ &queueCreateInfos ]( const auto queueFamilyID ) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = vk::StructureType::eDeviceQueueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamilyID;
        queueCreateInfo.queueCount       = queueCount;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.emplace_back( queueCreateInfo );
    } );

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = vk::True;

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType                   = vk::StructureType::eDeviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos       = std::data( queueCreateInfos );
    deviceCreateInfo.queueCreateInfoCount    = utils::size( queueCreateInfos );
    deviceCreateInfo.pEnabledFeatures        = nullptr;
    deviceCreateInfo.enabledExtensionCount   = utils::size( physicalDeviceExtensions );
    deviceCreateInfo.ppEnabledExtensionNames = std::data( physicalDeviceExtensions );
    deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;

    m_logicalDevice = m_physicalDevice.get().createDevice( deviceCreateInfo, nullptr );

    constexpr uint32_t queueIndex{ 0U };
    m_queues.emplace( ve::QueueType::eGraphics,
                      m_logicalDevice.getQueue( queueFamilyIndices.at( FamilyType::eGraphics ), queueIndex ) );
    m_queues.emplace( ve::QueueType::ePresentation,
                      m_logicalDevice.getQueue( queueFamilyIndices.at( FamilyType::ePresentation ), queueIndex ) );
    m_queues.emplace( ve::QueueType::eTransfer,
                      m_logicalDevice.getQueue( queueFamilyIndices.at( FamilyType::eTransfer ), queueIndex ) );
}

} // namespace ve
