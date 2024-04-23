#pragma once

#include "LogicalDevice.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"

#include <array>

using ShaderStageInfos = std::array< VkPipelineShaderStageCreateInfo, 2u >;

namespace ve {

struct PipelineConfigInfo {
    const VkPipelineDynamicStateCreateInfo dynamicState{};
    const VkPipelineViewportStateCreateInfo viewport{};
    const VkPipelineVertexInputStateCreateInfo vertexInput{};
    const VkPipelineInputAssemblyStateCreateInfo inputAsembly{};
    const VkPipelineRasterizationStateCreateInfo rasterizer{};
    const VkPipelineMultisampleStateCreateInfo multisampling{};
    const VkPipelineColorBlendStateCreateInfo colorBlends{};
};

class Pipeline {
public:
    Pipeline( const ve::LogicalDevice& logicalDevice, const ve::Swapchain& swapchain );

    ~Pipeline();

    VkPipeline getHandler() const;
    VkViewport getViewport() const;
    VkRect2D getScissor() const;

private:
    Shader m_vertexShader;
    Shader m_fragmentShader;
    const ve::LogicalDevice& m_logicalDevice;
    const ve::Swapchain& m_swapchain;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkViewport m_viewport;
    VkRect2D m_scissor;

    void createPipelineLayout();
    void createPipeline();

    VkPipelineShaderStageCreateInfo pupulateShaderStageInfo( enum VkShaderStageFlagBits shaderType,
                                                             const Shader& shader ) const;
    ShaderStageInfos createShaderStagesInfo() const;
    VkPipelineDynamicStateCreateInfo createDynamicStatesInfo() const;
    void createViewport();
    void createScissor();
    VkPipelineViewportStateCreateInfo createViewportStateInfo() const;
    VkPipelineVertexInputStateCreateInfo createVertexInputInfo() const;
    VkPipelineInputAssemblyStateCreateInfo createInputAsemblyInfo() const;
    VkPipelineRasterizationStateCreateInfo createRasterizerInfo() const;
    VkPipelineMultisampleStateCreateInfo createMultisamplingInfo() const;
    VkPipelineColorBlendAttachmentState createColorBlendAttachmentState() const;
    VkPipelineColorBlendStateCreateInfo
        createColorBlendAttachmentInfo( VkPipelineColorBlendAttachmentState state ) const;
};

}; // namespace ve