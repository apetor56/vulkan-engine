#pragma once

#include "utils/NonCopyable.hpp"
#include "utils/NonMovable.hpp"

#include <vk_mem_alloc.h>

namespace ve {

class VulkanInstance;
class PhysicalDevice;
class LogicalDevice;

class MemoryAllocator : public utils::NonCopyable,
                        public utils::NonMovable {
public:
    MemoryAllocator( const VulkanInstance& instance, const PhysicalDevice& physicalDevice,
                     const LogicalDevice& logicalDevice );

    ~MemoryAllocator();

    VmaAllocator get() const noexcept { return m_allocator; }

private:
    VmaAllocator m_allocator;
};

} // namespace ve
