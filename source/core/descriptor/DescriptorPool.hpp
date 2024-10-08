#pragma once

#include "LogicalDevice.hpp"
#include "DescriptorSetLayout.hpp"

#include <span>

namespace ve {

// TODO: management system for created sets count

class DescriptorPool {
public:
    DescriptorPool( const ve::LogicalDevice& logicalDevice, std::span< vk::DescriptorType > types,
                    const std::uint32_t descriptorCount, const std::uint32_t maxSetsCount );

    ~DescriptorPool();

    std::vector< vk::DescriptorSet > createDescriptorSets( const std::uint32_t count,
                                                           const ve::DescriptorSetLayout& layout );

private:
    vk::DescriptorPool m_descriptorPool;
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve