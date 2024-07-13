#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"

namespace ve {

class PhysicalDevice {
public:
	PhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window );

	PhysicalDevice( const PhysicalDevice& other ) = delete;
	PhysicalDevice( PhysicalDevice&& other )	  = delete;

	PhysicalDevice& operator=( const PhysicalDevice& other ) = delete;
	PhysicalDevice& operator=( PhysicalDevice&& other )		 = delete;

	VkPhysicalDevice getHandler() const;
	const extentions& getExtensions() const noexcept;

private:
	VkPhysicalDevice m_physicalDevice;
	const extentions m_deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	const ve::VulkanInstance& m_instance;
	const ve::Window& m_window;

	void pickPhysicalDevice();
	uint32_t rate( const VkPhysicalDevice device ) const;
	bool areRequiredExtensionsSupported( const VkPhysicalDevice physicalDevice ) const;
};

} // namespace ve