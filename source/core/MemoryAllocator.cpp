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
    allocatorCreateInfo.physicalDevice = physicalDevice.get();
    allocatorCreateInfo.device         = logicalDevice.get();

    vmaCreateAllocator( &allocatorCreateInfo, &m_allocator );
}

MemoryAllocator::~MemoryAllocator() {
    vmaDestroyAllocator( m_allocator );
}

} // namespace ve
