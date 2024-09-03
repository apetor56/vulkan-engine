#pragma once

#include "PhysicalDevice.hpp"

namespace ve {

class LogicalDevice {
public:
    LogicalDevice( const ve::PhysicalDevice& physicalDevice );

    LogicalDevice( const LogicalDevice& other ) = delete;
    LogicalDevice( LogicalDevice&& other )      = delete;

    LogicalDevice& operator=( const LogicalDevice& other ) = delete;
    LogicalDevice& operator=( LogicalDevice&& other )      = delete;

    ~LogicalDevice();

    vk::Device getHandler() const noexcept;
    vk::Queue getGraphicsQueue() const noexcept;
    vk::Queue getPresentationQueue() const noexcept;
    [[nodiscard]] ve::QueueFamilyIDs getQueueFamilyIDs() const noexcept;
    [[nodiscard]] vk::PhysicalDeviceMemoryProperties getMemoryProperties() const noexcept;

private:
    vk::Device m_logicalDevice;
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentationQueue;

    const ve::PhysicalDevice& m_physicalDevice;

    void createLogicalDevice();
};

} // namespace ve
