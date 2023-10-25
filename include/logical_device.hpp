#pragma once

#include "physical_device.hpp"

namespace VE {

class LogicalDevice {
public:
    LogicalDevice(std::shared_ptr<PhysicalDevice> physicalDevice,
                  std::shared_ptr<Window> window);

    LogicalDevice(const LogicalDevice& other) = delete;
    LogicalDevice(LogicalDevice&& other) = delete;

    LogicalDevice& operator=(const LogicalDevice& other) = delete;
    LogicalDevice& operator=(LogicalDevice&& other) = delete;

    ~LogicalDevice();

    VkDevice getHandle() const noexcept;

private:
    std::shared_ptr<PhysicalDevice> m_physicalDevice;
    std::shared_ptr<Window> m_window;
    VkDevice m_logicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    void createLogicalDevice();
};

}