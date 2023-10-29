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
    VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
    vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageCreateInfo.module = m_vertexShader.getModule();
    vertexStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStageCreateInfo{};
    fragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageCreateInfo.module = m_fragmentShader.getModule();
    fragmentStageCreateInfo.pName = "main";

    stage_infos shaderStages { vertexStageCreateInfo, fragmentStageCreateInfo };
}
}