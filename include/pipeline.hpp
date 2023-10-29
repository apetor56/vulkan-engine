#pragma once

#include "logical_device.hpp"
#include "shader.hpp"

#include <memory>

namespace VE {

struct PipelineConfigInfo {};

class Pipeline {
public:
    Pipeline(std::shared_ptr<LogicalDevice> logicalDevice);

private:
    Shader m_vertexShader;
    Shader m_fragmentShader;
    std::shared_ptr<LogicalDevice> m_logicalDevice;

    void createPipeline();

    VkPipelineShaderStageCreateInfo pupulateShaderStageInfo(enum VkShaderStageFlagBits shaderType, const Shader& shader) const;
};

};