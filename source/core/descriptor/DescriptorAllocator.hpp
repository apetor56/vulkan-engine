#pragma once

#include "DescriptorSetLayout.hpp"

namespace ve {

class DescriptorAllocator : public utils::NonCopyable,
                            public utils::NonMovable {
public:
    struct PoolSizeRatio {
        vk::DescriptorType descriptorType{};
        float ratio;
    };

    DescriptorAllocator( const ve::LogicalDevice& logicalDevice, const std::uint32_t maxSets,
                         std::span< PoolSizeRatio > poolSizeRatios );

    ~DescriptorAllocator();

    vk::DescriptorSet allocate( const ve::DescriptorSetLayout& layout, void *pNext = nullptr );
    void clearPools();
    void destroyPools();

private:
    using Pools = std::vector< vk::DescriptorPool >;

    Pools m_fullPools;
    Pools m_availablePools;
    std::vector< PoolSizeRatio > m_poolSizeRatios;
    const ve::LogicalDevice& m_logicalDevice;
    std::uint32_t m_setsPerPool{ 1U };

    vk::DescriptorPool getPool();
    vk::DescriptorPool createNewPool();
};

} // namespace ve
