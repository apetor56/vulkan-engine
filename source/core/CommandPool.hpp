#pragma once

#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"

namespace ve {

class CommandPool {
public:
    CommandPool( const ve::LogicalDevice& logicalDevice, const ve::Swapchain& swapchain, const ve::Pipeline& pipeline );
    ~CommandPool();

    CommandPool( const CommandPool& other ) = delete;
    CommandPool( CommandPool&& other )      = delete;

    CommandPool& operator=( const CommandPool& other ) = delete;
    CommandPool& operator=( CommandPool&& other )      = delete;

    ve::CommandBuffer createCommandBuffer();

private:
    vk::CommandPool m_commandPool;
    const ve::LogicalDevice& m_logicalDevice;
    const ve::Swapchain& m_swapchain;
    const ve::Pipeline& m_pipeline;

    void createCommandPool();
};

} // namespace ve
