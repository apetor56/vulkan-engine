#include "DescriptorAllocator.hpp"
#include "utils/Common.hpp"

namespace ve {

DescriptorAllocator::DescriptorAllocator( const ve::LogicalDevice& logicalDevice, const uint32_t maxSets,
                                          std::span< const PoolSizeRatio > poolSizeRatios )
    : m_logicalDevice{ logicalDevice }, m_setsPerPool{ static_cast< uint32_t >( maxSets * 1.5 ) } {
    assert( maxSets != 0 );
    m_poolSizeRatios.assign_range( poolSizeRatios );
    m_availablePools.emplace_back( createNewPool() );
}

DescriptorAllocator::~DescriptorAllocator() {
    destroyPools();
}

vk::DescriptorSet DescriptorAllocator::allocate( const ve::DescriptorSetLayout& layout, void *pNext ) {
    vk::DescriptorPool poolToUse{ getPool() };
    const auto layoutVk{ layout.get() };

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.pNext              = pNext;
    allocInfo.descriptorPool     = poolToUse;
    allocInfo.descriptorSetCount = 1U;
    allocInfo.pSetLayouts        = &layoutVk;

    std::optional< vk::DescriptorSet > descriptorSet{};
    try {
        descriptorSet = m_logicalDevice.get().allocateDescriptorSets( allocInfo ).at( 0 );
    }
    catch ( const vk::OutOfPoolMemoryError& ) {
        m_fullPools.emplace_back( poolToUse );
        poolToUse                = getPool();
        allocInfo.descriptorPool = poolToUse;
        descriptorSet            = m_logicalDevice.get().allocateDescriptorSets( allocInfo ).at( 0 );
    }
    catch ( const vk::FragmentedPoolError& ) {
        m_fullPools.emplace_back( poolToUse );
        poolToUse                = getPool();
        allocInfo.descriptorPool = poolToUse;
        descriptorSet            = m_logicalDevice.get().allocateDescriptorSets( allocInfo ).at( 0 );
    }

    if ( !descriptorSet.has_value() )
        throw std::runtime_error( "failed to create descriptor set" );

    m_availablePools.emplace_back( poolToUse );
    return descriptorSet.value();
}

void DescriptorAllocator::clearPools() {
    std::ranges::for_each( m_availablePools,
                           [ this ]( auto& pool ) { m_logicalDevice.get().resetDescriptorPool( pool ); } );

    m_availablePools.reserve( std::size( m_availablePools ) + std::size( m_fullPools ) );

    std::ranges::for_each( m_fullPools, [ this ]( auto& pool ) {
        m_logicalDevice.get().resetDescriptorPool( pool );
        m_availablePools.emplace_back( pool );
    } );
    m_fullPools.clear();
}

void DescriptorAllocator::destroyPools() {
    auto destroyPool{ [ this ]( auto& poolContainer ) {
        std::ranges::for_each( poolContainer,
                               [ this ]( auto& pool ) { m_logicalDevice.get().destroyDescriptorPool( pool ); } );
        poolContainer.clear();
    } };

    destroyPool( m_availablePools );
    destroyPool( m_fullPools );
}

vk::DescriptorPool DescriptorAllocator::getPool() {
    vk::DescriptorPool newPool{};
    if ( std::size( m_availablePools ) != 0U ) {
        newPool = m_availablePools.back();
        m_availablePools.pop_back();
    } else {
        newPool = createNewPool();
        m_setsPerPool *= 1.5F;
        if ( m_setsPerPool > 4092U )
            m_setsPerPool = 4092U;
    }

    return newPool;
}

vk::DescriptorPool DescriptorAllocator::createNewPool() {
    std::vector< vk::DescriptorPoolSize > poolSizes{};
    poolSizes.reserve( std::size( m_poolSizeRatios ) );
    std::ranges::for_each( m_poolSizeRatios, [ &poolSizes, this ]( auto& poolSizeRatio ) {
        poolSizes.emplace_back( poolSizeRatio.descriptorType, poolSizeRatio.ratio * m_setsPerPool );
    } );

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = vk::StructureType::eDescriptorPoolCreateInfo;
    poolInfo.maxSets       = m_setsPerPool;
    poolInfo.pPoolSizes    = std::data( poolSizes );
    poolInfo.poolSizeCount = utils::size( poolSizes );

    return m_logicalDevice.get().createDescriptorPool( poolInfo );
}

} // namespace ve
