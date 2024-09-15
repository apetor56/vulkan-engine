#include "QueueFamilyIDs.hpp"

namespace ve {

bool QueueFamilyIDs::hasRequiredFamilies() const noexcept {
    return m_familyIndices.contains( FamilyType::eGraphics ) && m_familyIndices.contains( FamilyType::ePresentation ) &&
           m_familyIndices.contains( FamilyType::eTransfer );
}

QueueFamilyIDs QueueFamilyIDs::findQueueFamilies( const vk::PhysicalDevice device, const vk::SurfaceKHR surface ) {
    QueueFamilyIDs queueFamilyIndices{};
    const auto queueFamilyProperties{ device.getQueueFamilyProperties() };

    std::uint32_t queueFamilyID{};
    vk::Bool32 isPresentionSupportAvailable{ false };

    for ( const auto& queueFamily : queueFamilyProperties ) {
        isPresentionSupportAvailable = device.getSurfaceSupportKHR( queueFamilyID, surface );

        if ( queueFamily.queueFlags & vk::QueueFlagBits::eGraphics )
            queueFamilyIndices.add( FamilyType::eGraphics, queueFamilyID );

        if ( isPresentionSupportAvailable )
            queueFamilyIndices.add( FamilyType::ePresentation, queueFamilyID );

        if ( ( queueFamily.queueFlags & vk::QueueFlagBits::eTransfer ) &&
             !( queueFamily.queueFlags & vk::QueueFlagBits::eGraphics ) )
            queueFamilyIndices.add( FamilyType::eTransfer, queueFamilyID );

        if ( queueFamilyIndices.hasRequiredFamilies() )
            break;

        queueFamilyID++;
    }

    return queueFamilyIndices;
}

void QueueFamilyIDs::add( FamilyType type, std::uint32_t familyID ) {
    m_familyIndices.emplace( type, familyID );
}

std::unordered_map< FamilyType, std::uint32_t > QueueFamilyIDs::getAll() const {
    return m_familyIndices;
}

} // namespace ve
