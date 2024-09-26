#define VMA_IMPLEMENTATION

#include "MemoryAllocator.hpp"
#include "VulkanInstance.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"

namespace ve {

MemoryAllocator::MemoryAllocator( const VulkanInstance& instance, const PhysicalDevice& physicalDevice,
                                  const LogicalDevice& logicalDevice ) {
    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.instance       = instance.get();
    allocatorCreateInfo.physicalDevice = physicalDevice.getHandler();
    allocatorCreateInfo.device         = logicalDevice.getHandler();

    vmaCreateAllocator( &allocatorCreateInfo, &m_allocator );
}

MemoryAllocator::~MemoryAllocator() {
    vmaDestroyAllocator( m_allocator );
}

MemoryAllocator::operator VmaAllocator() const noexcept {
    return m_allocator;
}

} // namespace ve