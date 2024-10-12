#include "SyncObjects.hpp"

namespace ve {

Fence::Fence( const ve::LogicalDevice& logicalDevice, const vk::FenceCreateFlagBits createFlags )
    : m_logicalDevice{ logicalDevice } {
    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = createFlags;

    m_fence = m_logicalDevice.getHandler().createFence( fenceInfo );
}

Fence::~Fence() {
    m_logicalDevice.getHandler().destroyFence( m_fence );
}

Semaphore::Semaphore( const ve::LogicalDevice& logicalDevice, const vk::SemaphoreCreateFlagBits createFlags )
    : m_logicalDevice{ logicalDevice } {
    vk::SemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = createFlags;

    m_semaphore = m_logicalDevice.getHandler().createSemaphore( semaphoreInfo );
}

Semaphore::~Semaphore() {
    m_logicalDevice.getHandler().destroySemaphore( m_semaphore );
}

} // namespace ve