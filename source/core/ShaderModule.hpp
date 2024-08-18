#pragma once

#include "LogicalDevice.hpp"

namespace ve {

class ShaderModule {
public:
    ShaderModule( std::string_view shaderBinaryPath, const ve::LogicalDevice& logicalDevice );
    ~ShaderModule();

    ShaderModule( const ShaderModule& other ) = delete;
    ShaderModule( ShaderModule&& other )      = delete;

    ShaderModule& operator=( const ShaderModule& other ) = delete;
    ShaderModule& operator=( ShaderModule&& other )      = delete;

    vk::ShaderModule getHandler() const noexcept;

private:
    vk::ShaderModule m_shaderModule{};
    const ve::LogicalDevice& m_logicalDevice;

    std::vector< std::byte > getShaderBinaryCode( std::string_view shaderBinaryPath ) const;
    void createShaderModule( const std::vector< std::byte >& shaderByteCode );
};

} // namespace ve
