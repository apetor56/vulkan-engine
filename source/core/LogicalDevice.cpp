#include "LogicalDevice.hpp"
#include "QueueFamilyIndices.hpp"

#include <set>
#include <stdexcept>

namespace ve {

LogicalDevice::LogicalDevice( const ve::PhysicalDevice& physicalDevice, const ve::Window& window )
	: m_physicalDevice{ physicalDevice }, m_window{ window } {
	createLogicalDevice();
}

LogicalDevice::~LogicalDevice() {
	vkDestroyDevice( m_logicalDevice, nullptr );
}

void LogicalDevice::createLogicalDevice() {
	const auto physicalDeviceHandle{ m_physicalDevice.getHandler() };
	const auto surface{ m_window.getSurface() };
	const auto& extensions{ m_physicalDevice.getExtensions() };

	QueueFamilyIndices queueFamilyIndices{ QueueFamilyIndices::findQueueFamilies( physicalDeviceHandle, surface ) };
	constexpr uint32_t queueCount{ 1u };
	constexpr float queuePriority{ 1.f };

	std::vector< VkDeviceQueueCreateInfo > queueCreateInfos{};
	std::set< uint32_t > uniqueQueueFamilies{ queueFamilyIndices.graphicsFamily.value(),
											  queueFamilyIndices.presentFamily.value() };

	for ( const auto& queueFamily : uniqueQueueFamilies ) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType			 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount		 = queueCount;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.emplace_back( queueCreateInfo );
	}

	VkPhysicalDeviceFeatures deviceFeatures{}; // empty for now

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType					 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos		 = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount	 = static_cast< uint32_t >( std::size( queueCreateInfos ) );
	deviceCreateInfo.pEnabledFeatures		 = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount	 = static_cast< uint32_t >( std::size( extensions ) );
	deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

	if ( vkCreateDevice( physicalDeviceHandle, &deviceCreateInfo, nullptr, &m_logicalDevice ) != VK_SUCCESS ) {
		throw std::runtime_error( "failed to create logical device" );
	}

	constexpr uint32_t queueIndex{};
	vkGetDeviceQueue( m_logicalDevice, queueFamilyIndices.graphicsFamily.value(), queueIndex, &m_graphicsQueue );
	vkGetDeviceQueue( m_logicalDevice, queueFamilyIndices.presentFamily.value(), queueIndex, &m_presentQueue );
}

VkDevice LogicalDevice::getHandler() const noexcept {
	return m_logicalDevice;
}

VkQueue LogicalDevice::getGraphicsQueue() const noexcept {
	return m_graphicsQueue;
}

VkQueue LogicalDevice::getPresentationQueue() const noexcept {
	return m_presentQueue;
}

} // namespace ve