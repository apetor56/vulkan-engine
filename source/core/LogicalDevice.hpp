#pragma once

#include "PhysicalDevice.hpp"

#include <unordered_map>

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
    vk::Queue getQueue( const ve::QueueType queueType ) const;
    [[nodiscard]] std::unordered_map< ve::FamilyType, std::uint32_t > getQueueFamilyIDs() const noexcept;
    [[nodiscard]] vk::PhysicalDeviceMemoryProperties getMemoryProperties() const noexcept;

private:
    vk::Device m_logicalDevice;
    std::unordered_map< ve::QueueType, vk::Queue > m_queues;

    const ve::PhysicalDevice& m_physicalDevice;

    void createLogicalDevice();
};

} // namespace ve
