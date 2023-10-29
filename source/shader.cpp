#include "shader.hpp"
#include "logical_device.hpp"

namespace VE {

Shader::Shader(std::string_view shaderPath, std::shared_ptr<LogicalDevice> logicalDevice) : m_logicalDevice { logicalDevice } {
    const auto& shaderCode { readShaderBinary(shaderPath) };
    createShaderModule(shaderCode);
}

Shader::~Shader() {
    vkDestroyShaderModule(m_logicalDevice->getHandle(), m_shaderModule, nullptr);
}

std::vector<char> Shader::readShaderBinary(std::string_view shaderPath) const {
    std::ifstream shaderFile { shaderPath.data(), std::ios::ate | std::ios::binary };

    if (!shaderFile.is_open()) {
        throw std::runtime_error("failed to open file");
    }

    const size_t shaderSize { static_cast<size_t>(shaderFile.tellg()) };
    std::vector<char> shaderByteCode(shaderSize);

    shaderFile.seekg(0);
    shaderFile.read(shaderByteCode.data(), shaderSize);
    shaderFile.close();

    return shaderByteCode;
}

void Shader::createShaderModule(const std::vector<char>& shaderByteCode) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = std::size(shaderByteCode);
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(shaderByteCode.data());

    if(vkCreateShaderModule(m_logicalDevice->getHandle(), &createInfo, nullptr, &m_shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }
}

VkShaderModule Shader::getModule() const {
    return m_shaderModule;
}

}