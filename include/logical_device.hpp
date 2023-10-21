#pragma once

#include "physical_device.hpp"

namespace VE {

class LogicalDevice {
public:
    LogicalDevice(std::shared_ptr<PhysicalDevice> physicalDevice,
                  std::shared_ptr<Window> window);

    ~LogicalDevice();

    VkDevice getHandle() const;

private:
    std::shared_ptr<PhysicalDevice> m_physicalDevice;
    std::shared_ptr<Window> m_window;
    VkDevice m_logicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    void createLogicalDevice();
};

}