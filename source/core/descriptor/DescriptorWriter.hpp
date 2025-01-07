#pragma once

#include "LogicalDevice.hpp"

#include <deque>

namespace ve {

class DescriptorWriter : public utils::NonCopyable,
                         public utils::NonMovable {
public:
    DescriptorWriter( const ve::LogicalDevice& logicalDevice );

    void writeImage( const std::uint32_t binding, const vk::ImageView imageView, const vk::ImageLayout imageLayout,
                     const vk::Sampler sampler, const vk::DescriptorType descriptorType );
    void writeBuffer( const std::uint32_t binding, const vk::Buffer buffer, const std::uint32_t range,
                      const std::size_t offset, const vk::DescriptorType descriptorType );
    void clear();
    void updateSet( const vk::DescriptorSet set );

private:
    std::deque< vk::DescriptorImageInfo > m_descriptorImageInfos{};
    std::deque< vk::DescriptorBufferInfo > m_descriptorBufferInfos{};
    std::vector< vk::WriteDescriptorSet > m_descriptorWrites{};
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve
