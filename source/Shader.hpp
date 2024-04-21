#pragma once

#include "LogicalDevice.hpp"

#include <vulkan/vulkan.h>

#include <string_view>
#include <fstream>
#include <vector>
#include <memory>

namespace ve {

class Shader {
public:
    Shader( std::string_view shaderPath, const ve::LogicalDevice& logicalDevice );
    ~Shader();

    VkShaderModule getModule() const;

private:
    VkShaderModule m_shaderModule;
    const ve::LogicalDevice& m_logicalDevice;

    std::vector< char > readShaderBinary( std::string_view shaderPath ) const;
    void createShaderModule( const std::vector< char >& shaderByteCode );
};

} // namespace ve