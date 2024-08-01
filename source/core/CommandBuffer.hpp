#pragma once

#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "Window.hpp"

namespace ve {

class CommandBuffer {
public:
    CommandBuffer( const ve::PhysicalDevice& physicalDevice, const ve::LogicalDevice& logicalDevice,
                   const ve::Swapchain& swapchain, const ve::Pipeline& pipeline );

    ~CommandBuffer();

    void record( const uint32_t imageIndex ) const;
    void reset();
    VkCommandBuffer getHandler() const noexcept;

private:
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    const ve::PhysicalDevice& m_physicalDevice;
    const ve::LogicalDevice& m_logicalDevice;
    const ve::Swapchain& m_swapchain;
    const ve::Pipeline& m_pipeline;

    void createCommandPool();
    void createCommandBuffer();
};

} // namespace ve
