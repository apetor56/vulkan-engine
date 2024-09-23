#include "DescriptorPool.hpp"

namespace ve {

DescriptorPool::DescriptorPool( const ve::LogicalDevice& logicalDevice, const vk::DescriptorType type,
                                const std::uint32_t descriptorCount, const std::uint32_t maxSetsCount )
    : m_logicalDevice{ logicalDevice } {
    vk::DescriptorPoolSize poolSize;
    poolSize.type            = type;
    poolSize.descriptorCount = descriptorCount;

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = vk::StructureType::eDescriptorPoolCreateInfo;
    poolInfo.poolSizeCount = 1U;
    poolInfo.pPoolSizes    = &poolSize;
    poolInfo.maxSets       = maxSetsCount;

    m_descriptorPool = m_logicalDevice.getHandler().createDescriptorPool( poolInfo );
}

DescriptorPool::~DescriptorPool() {
    m_logicalDevice.getHandler().destroyDescriptorPool( m_descriptorPool );
}

std::vector< vk::DescriptorSet > DescriptorPool::createDescriptorSets( const std::uint32_t count,
                                                                       const ve::DescriptorSetLayout& layout ) {
    const std::vector< vk::DescriptorSetLayout > layouts( count, layout.getHandler() );

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool     = m_descriptorPool;
    allocInfo.descriptorSetCount = count;
    allocInfo.pSetLayouts        = std::data( layouts );

    return m_logicalDevice.getHandler().allocateDescriptorSets( allocInfo );
}

} // namespace ve