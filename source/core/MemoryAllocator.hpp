#pragma once

#include "vma/vk_mem_alloc.h"

namespace ve {

class VulkanInstance;
class PhysicalDevice;
class LogicalDevice;

class MemoryAllocator {
public:
    MemoryAllocator( const VulkanInstance& instance, const PhysicalDevice& physicalDevice,
                     const LogicalDevice& logicalDevice );

    ~MemoryAllocator();

    operator VmaAllocator() const noexcept;

private:
    VmaAllocator m_allocator;
};

} // namespace ve