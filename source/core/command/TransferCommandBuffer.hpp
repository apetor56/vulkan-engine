#pragma once

#include "BaseCommandBuffer.hpp"

namespace ve {

class LogicalDevice;

class TransferCommandBuffer : public BaseCommandBuffer {
public:
    using BaseCommandBuffer::BaseCommandBuffer;
    using BaseCommandBuffer::operator=;

    static std::uint32_t getQueueFamilyID( const ve::LogicalDevice& logicalDevice );

    void copyBuffer( const vk::DeviceSize srcOffset, const vk::DeviceSize dstOffset, const std::size_t size,
                     const vk::Buffer srcBuffer, const vk::Buffer dstBuffer ) const;
};

} // namespace ve
