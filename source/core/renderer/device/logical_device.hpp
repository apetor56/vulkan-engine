#pragma once

#include "PhysicalDevice.hpp"

#include <unordered_map>

namespace ve {

class LogicalDevice : public utils::NonCopyable,
                      public utils::NonMovable {
public:
    LogicalDevice( const ve::PhysicalDevice& physicalDevice );
    ~LogicalDevice();

    vk::Device get() const noexcept { return m_logicalDevice; }
    vk::Queue getQueue( const ve::QueueType queueType ) const { return m_queues.at( queueType ); }
    [[nodiscard]] ve::QueueFamilyMap getQueueFamilyIDs() const noexcept { return m_physicalDevice.getQueueFamilyIDs(); }

    const ve::PhysicalDevice& getParentPhysicalDevice() const noexcept { return m_physicalDevice; }

private:
    std::unordered_map< ve::QueueType, vk::Queue > m_queues;
    vk::Device m_logicalDevice;
    const ve::PhysicalDevice& m_physicalDevice;

    void createLogicalDevice();
};

} // namespace ve
