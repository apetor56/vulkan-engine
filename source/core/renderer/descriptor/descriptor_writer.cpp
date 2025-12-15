#include "DescriptorWriter.hpp"

namespace ve {

DescriptorWriter::DescriptorWriter( const ve::LogicalDevice& logicalDevice ) : m_logicalDevice{ logicalDevice } {}

void DescriptorWriter::writeImage( const uint32_t binding, const vk::ImageView imageView,
                                   const vk::ImageLayout imageLayout, const vk::Sampler sampler,
                                   const vk::DescriptorType descriptorType ) {
    const vk::DescriptorImageInfo& descriptorImageInfo{
        m_descriptorImageInfos.emplace_back( sampler, imageView, imageLayout ) };

    vk::WriteDescriptorSet write;
    write.sType           = vk::StructureType::eWriteDescriptorSet;
    write.dstBinding      = binding;
    write.dstSet          = nullptr;
    write.descriptorCount = 1U;
    write.descriptorType  = descriptorType;
    write.pImageInfo      = &descriptorImageInfo;

    m_descriptorWrites.emplace_back( write );
}

void DescriptorWriter::writeBuffer( const uint32_t binding, const vk::Buffer buffer, const uint32_t range,
                                    const std::size_t offset, const vk::DescriptorType descriptorType ) {
    const vk::DescriptorBufferInfo& descriptorBufferInfo{
        m_descriptorBufferInfos.emplace_back( buffer, offset, range ) };

    vk::WriteDescriptorSet write;
    write.sType           = vk::StructureType::eWriteDescriptorSet;
    write.dstBinding      = binding;
    write.dstSet          = nullptr;
    write.descriptorCount = 1U;
    write.descriptorType  = descriptorType;
    write.pBufferInfo     = &descriptorBufferInfo;

    m_descriptorWrites.emplace_back( write );
}

void DescriptorWriter::clear() {
    m_descriptorImageInfos.clear();
    m_descriptorBufferInfos.clear();
    m_descriptorWrites.clear();
}

void DescriptorWriter::updateSet( const vk::DescriptorSet set ) {
    std::ranges::for_each( m_descriptorWrites, [ &set ]( auto& write ) { write.dstSet = set; } );
    m_logicalDevice.get().updateDescriptorSets( m_descriptorWrites, nullptr );
}

} // namespace ve
