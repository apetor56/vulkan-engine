#pragma once

#include "LogicalDevice.hpp"

#include <filesystem>

namespace ve {

class ShaderModule : public utils::NonCopyable,
                     public utils::NonMovable {
public:
    ShaderModule( const std::filesystem::path& shaderBinaryPath, const ve::LogicalDevice& logicalDevice );
    ~ShaderModule();

    vk::ShaderModule get() const noexcept;

private:
    vk::ShaderModule m_shaderModule{};
    const ve::LogicalDevice& m_logicalDevice;

    std::vector< std::byte > getShaderBinaryCode( const std::filesystem::path& shaderBinaryPath ) const;
    void createShaderModule( const std::vector< std::byte >& shaderByteCode );
};

} // namespace ve
