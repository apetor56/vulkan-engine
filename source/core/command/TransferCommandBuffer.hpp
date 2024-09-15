#pragma once

#include "BaseCommandBuffer.hpp"

namespace ve {

class TransferCommandBuffer : public BaseCommandBuffer {
public:
    using BaseCommandBuffer::BaseCommandBuffer;
    using BaseCommandBuffer::operator=;

    void copyBuffer( const vk::Buffer stagingBuffer, const vk::Buffer actualBuffer, const std::size_t size ) const;
};

} // namespace ve
