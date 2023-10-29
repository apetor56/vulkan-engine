#include "pipeline.hpp"
#include "config.hpp"

#include <array>

using stage_infos = std::array<VkPipelineShaderStageCreateInfo, 2u>;

namespace VE {
Pipeline::Pipeline(std::shared_ptr<LogicalDevice> logicalDevice) :
            m_vertexShader { cfg::shader::vertShaderBinaryPath, logicalDevice },
            m_fragmentShader { cfg::shader::fragShaderBinaryPath, logicalDevice },
            m_logicalDevice { logicalDevice } {
    createPipeline();
}

void Pipeline::createPipeline() {
    const auto vertStageInfo { pupulateShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, m_vertexShader) };
    const auto fragStageInfo { pupulateShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, m_fragmentShader) };

    stage_infos shaderStages { vertStageInfo, fragStageInfo };
}

VkPipelineShaderStageCreateInfo Pipeline::pupulateShaderStageInfo(enum VkShaderStageFlagBits shaderType, const Shader& shader) const {
    VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
    vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageCreateInfo.stage = shaderType;
    vertexStageCreateInfo.module = shader.getModule();
    vertexStageCreateInfo.pName = "main";

    return vertexStageCreateInfo;
}
}