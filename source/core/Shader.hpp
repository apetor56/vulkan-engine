#pragma once

#include "LogicalDevice.hpp"

#include <vulkan/vulkan.h>

#include <string_view>
#include <vector>

namespace ve {

using shaderCode = std::vector< char >;

class Shader {
public:
	Shader( std::string_view shaderPath, const ve::LogicalDevice& logicalDevice );
	~Shader();

	VkShaderModule getModule() const;

private:
	VkShaderModule m_shaderModule;
	const ve::LogicalDevice& m_logicalDevice;

	shaderCode readShaderBinary( std::string_view shaderPath ) const;
	void createShaderModule( const shaderCode& shaderByteCode );
};

} // namespace ve