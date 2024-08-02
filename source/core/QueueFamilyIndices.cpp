#include "QueueFamilyIndices.hpp"

namespace ve {

bool QueueFamilyIndices::hasRequiredFamilies() const noexcept {
    return graphicsFamilyID.has_value() && presentationFamilyID.has_value();
}

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies( const vk::PhysicalDevice device,
                                                          const vk::SurfaceKHR surface ) {
    QueueFamilyIndices queueFamilyIndices{};
    const auto queueFamilyProperties{ device.getQueueFamilyProperties() };

    std::uint32_t queueFamilyID{};
    vk::Bool32 isPresentionSupportAvailable{ false };

    for ( const auto& queueFamily : queueFamilyProperties ) {
        isPresentionSupportAvailable = device.getSurfaceSupportKHR( queueFamilyID, surface );

        if ( queueFamily.queueFlags & vk::QueueFlagBits::eGraphics )
            queueFamilyIndices.graphicsFamilyID = queueFamilyID;

        if ( isPresentionSupportAvailable )
            queueFamilyIndices.presentationFamilyID = queueFamilyID;

        if ( queueFamilyIndices.hasRequiredFamilies() )
            break;

        queueFamilyID++;
    }

    return queueFamilyIndices;
}

} // namespace ve
