#pragma once

#include "logical_device.hpp"

#include <vulkan/vulkan.h>
#include <string_view>
#include <fstream>
#include <vector>
#include <memory>

namespace VE {

class Shader {
public:
    Shader(std::string_view shaderPath, std::shared_ptr<LogicalDevice> logicalDevice);

    ~Shader();

    VkShaderModule getModule() const;

private:
    VkShaderModule m_shaderModule;
    std::shared_ptr<LogicalDevice> m_logicalDevice;

    std::vector<char> readShaderBinary(std::string_view shaderPath) const;

    void createShaderModule(const std::vector<char>& shaderByteCode);
};

}