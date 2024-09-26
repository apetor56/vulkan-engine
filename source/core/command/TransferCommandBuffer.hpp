#pragma once

#include "BaseCommandBuffer.hpp"

namespace ve {

class LogicalDevice;

class TransferCommandBuffer : public BaseCommandBuffer {
public:
    using BaseCommandBuffer::BaseCommandBuffer;
    using BaseCommandBuffer::operator=;

    static std::uint32_t getQueueFamilyID( const ve::LogicalDevice& logicalDevice );

    void copyBuffer( const vk::Buffer stagingBuffer, const vk::Buffer actualBuffer, const std::size_t size ) const;
};

} // namespace ve
