#pragma once

#include "physical_device.hpp"
#include "logical_device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"

namespace VE {

class CommandBuffer {
public:
    CommandBuffer(std::shared_ptr<PhysicalDevice> physicalDevice,
                  std::shared_ptr<LogicalDevice> logicalDevice,
                  std::shared_ptr<Swapchain> swapchain,
                  std::shared_ptr<Pipeline> pipeline);

    ~CommandBuffer();

    void record(const uint32_t imageIndex) const;

    void reset();

    VkCommandBuffer getHandle() const noexcept;

private:
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    std::shared_ptr<PhysicalDevice> m_physicalDevice;
    std::shared_ptr<LogicalDevice> m_logicalDevice;
    std::shared_ptr<Swapchain> m_swapchain;
    std::shared_ptr<Pipeline> m_pipeline;

    void createCommandPool();

    void createCommandBuffer();
};

}