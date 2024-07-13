#pragma once

#include "PhysicalDevice.hpp"

namespace ve {

class LogicalDevice {
public:
	LogicalDevice( const ve::PhysicalDevice& physicalDevice, const ve::Window& window );

	LogicalDevice( const LogicalDevice& other ) = delete;
	LogicalDevice( LogicalDevice&& other )		= delete;

	LogicalDevice& operator=( const LogicalDevice& other ) = delete;
	LogicalDevice& operator=( LogicalDevice&& other )	   = delete;

	~LogicalDevice();

	VkDevice getHandler() const noexcept;
	VkQueue getGraphicsQueue() const noexcept;
	VkQueue getPresentationQueue() const noexcept;

private:
	VkDevice m_logicalDevice;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	const ve::PhysicalDevice& m_physicalDevice;
	const ve::Window& m_window;

	void createLogicalDevice();
};

} // namespace ve