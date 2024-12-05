#pragma once

#include "LogicalDevice.hpp"
#include "ShaderModule.hpp"
#include "Swapchain.hpp"
#include "descriptor/DescriptorSetLayout.hpp"

#include <array>

using ShaderStageInfos = std::array< vk::PipelineShaderStageCreateInfo, 2u >;

namespace ve {

struct PipelineConfigInfo {
    const vk::PipelineDynamicStateCreateInfo dynamicState{};
    const vk::PipelineViewportStateCreateInfo viewportState{};
    const vk::PipelineVertexInputStateCreateInfo vertexInputState{};
    const vk::PipelineInputAssemblyStateCreateInfo inputAsemblyState{};
    const vk::PipelineRasterizationStateCreateInfo rasterizerState{};
    const vk::PipelineMultisampleStateCreateInfo multisamplingState{};
    const vk::PipelineColorBlendStateCreateInfo colorBlendsState{};
    const vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
};

class Pipeline {
public:
    Pipeline( const ve::LogicalDevice& logicalDevice, const ve::Swapchain& swapchain,
              const ve::DescriptorSetLayout& descriptorSetLayout );

    Pipeline( const Pipeline& other ) = delete;
    Pipeline( Pipeline&& other )      = delete;

    Pipeline& operator=( const Pipeline& other ) = delete;
    Pipeline& operator=( Pipeline&& other )      = delete;

    ~Pipeline();

    vk::Pipeline get() const noexcept;
    vk::PipelineLayout getLayout() const noexcept;

private:
    ve::ShaderModule m_vertexShader;
    ve::ShaderModule m_fragmentShader;
    vk::Pipeline m_graphicsPipeline;
    vk::PipelineLayout m_pipelineLayout;
    const ve::LogicalDevice& m_logicalDevice;
    const ve::Swapchain& m_swapchain;

    void createPipelineLayout( const ve::DescriptorSetLayout& descriptorLayout );
    void createPipeline();

    vk::PipelineShaderStageCreateInfo pupulateShaderStageInfo( const vk::ShaderStageFlagBits shaderType,
                                                               const ve::ShaderModule& shaderModule ) const;
    ShaderStageInfos createShaderStagesInfo() const;
    vk::PipelineDynamicStateCreateInfo createDynamicStatesInfo() const;
    vk::PipelineViewportStateCreateInfo createViewportStateInfo() const noexcept;
    vk::PipelineVertexInputStateCreateInfo createVertexInputInfo() const noexcept;
    vk::PipelineInputAssemblyStateCreateInfo createInputAsemblyInfo() const noexcept;
    vk::PipelineRasterizationStateCreateInfo createRasterizerInfo() const noexcept;
    vk::PipelineMultisampleStateCreateInfo createMultisamplingInfo() const noexcept;
    vk::PipelineColorBlendAttachmentState createColorBlendAttachmentState() const noexcept;
    vk::PipelineColorBlendStateCreateInfo
        createColorBlendStateInfo( const vk::PipelineColorBlendAttachmentState state ) const noexcept;
    vk::PipelineDepthStencilStateCreateInfo createDepthStencilInfo() const noexcept;
};

}; // namespace ve
