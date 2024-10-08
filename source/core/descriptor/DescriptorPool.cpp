#include "DescriptorPool.hpp"

namespace ve {

DescriptorPool::DescriptorPool( const ve::LogicalDevice& logicalDevice, std::span< vk::DescriptorType > types,
                                const std::uint32_t descriptorCount, const std::uint32_t maxSetsCount )
    : m_logicalDevice{ logicalDevice } {
    std::vector< vk::DescriptorPoolSize > poolSizes( std::size( types ) );
    for ( std::size_t index{}; index < std::size( types ); index++ ) {
        poolSizes.at( index ).type            = types[ index ];
        poolSizes.at( index ).descriptorCount = descriptorCount;
    }

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = vk::StructureType::eDescriptorPoolCreateInfo;
    poolInfo.poolSizeCount = static_cast< std::uint32_t >( std::size( poolSizes ) );
    poolInfo.pPoolSizes    = std::data( poolSizes );
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