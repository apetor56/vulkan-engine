#pragma once

#include "LogicalDevice.hpp"

namespace ve {

class Fence {
public:
    Fence( const ve::LogicalDevice& logicalDevice,
           const vk::FenceCreateFlagBits createFlags = vk::FenceCreateFlagBits::eSignaled );
    ~Fence();

    vk::Fence get() const noexcept { return m_fence; }

private:
    vk::Fence m_fence;
    const ve::LogicalDevice& m_logicalDevice;
};

class Semaphore {
public:
    Semaphore( const ve::LogicalDevice& logicalDevice, const vk::SemaphoreCreateFlagBits createFlags = {} );
    ~Semaphore();

    vk::Semaphore get() const noexcept { return m_semaphore; }

private:
    vk::Semaphore m_semaphore;
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve