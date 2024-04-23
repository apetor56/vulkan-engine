#include "QueueFamilyIndices.hpp"

#include <vector>

namespace ve {

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies( const VkPhysicalDevice device, const VkSurfaceKHR surface ) {
    QueueFamilyIndices queueFamilyIndices{};

    uint32_t queueFamilyCount{};
    vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

    std::vector< VkQueueFamilyProperties > queueFamilies( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );

    uint32_t queueFamilyIndex{};
    VkBool32 isPresentSupportAvailable{ false };

    for ( const auto& queueFamily : queueFamilies ) {
        if ( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
            queueFamilyIndices.graphicsFamily = queueFamilyIndex;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR( device, queueFamilyIndex, surface, &isPresentSupportAvailable );

        if ( isPresentSupportAvailable ) {
            queueFamilyIndices.presentFamily = queueFamilyIndex;
        }

        if ( queueFamilyIndices.isComplete() ) {
            break;
        }

        queueFamilyIndex++;
    }

    return queueFamilyIndices;
}

} // namespace ve