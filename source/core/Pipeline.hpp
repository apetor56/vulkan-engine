#pragma once

#include "LogicalDevice.hpp"
#include "ShaderModule.hpp"
#include "descriptor/DescriptorSetLayout.hpp"

#include <array>

namespace ve {

class PipelineBuilder;
class RenderPass;

class PipelineLayout : public utils::NonCopyable,
                       public utils::NonMovable {
public:
    PipelineLayout( const ve::LogicalDevice& logicalDevice, const ve::DescriptorSetLayout& descriptorLayout );
    ~PipelineLayout();

    vk::PipelineLayout get() const noexcept { return m_pipelineLayout; }

private:
    vk::PipelineLayout m_pipelineLayout;
    const ve::LogicalDevice& m_logicalDevice;
};

class Pipeline : public utils::NonCopyable,
                 public utils::NonMovable {
public:
    Pipeline( const PipelineBuilder& builder, const ve::RenderPass& renderPass );
    ~Pipeline();

    vk::Pipeline get() const noexcept { return m_graphicsPipeline; }
    vk::PipelineLayout getLayout() const noexcept { return m_layout; }

private:
    vk::Pipeline m_graphicsPipeline;
    vk::PipelineLayout m_layout;
    const ve::LogicalDevice& m_logicalDevice;
};

class PipelineBuilder : public utils::NonCopyable,
                        public utils::NonMovable {
public:
    using ShaderStageInfos = std::array< vk::PipelineShaderStageCreateInfo, 2u >;

    PipelineBuilder( const ve::LogicalDevice& logicalDevice );

    PipelineBuilder( const ve::LogicalDevice& logicalDevice, const ve::ShaderModule& vertexShader,
                     const ve::ShaderModule& fragmentShader, const ve::DescriptorSetLayout& descriptorLayout );

    void setShaders( const ve::ShaderModule& vertexShader, const ve::ShaderModule& fragmentShader );
    void setLayout( const ve::DescriptorSetLayout& descriptorLayout );

    [[nodiscard]] ve::Pipeline build( const ve::RenderPass& renderPass );

    const auto& getDepthStencilState() const noexcept { return m_depthStencilState; }
    const auto& getRasterizerState() const noexcept { return m_rasterizerState; }
    const auto& getColorBlendState() const noexcept { return m_colorBlendsState; }
    const auto& getMultisamplingState() const noexcept { return m_multisamplingState; }
    const auto& getViewportState() const noexcept { return m_viewportState; }
    const auto& getShaderStages() const noexcept { return m_shaderStages; }
    const auto& getDynamicState() const noexcept { return m_dynamicState; }
    const auto& getVertexInputState() const noexcept { return m_vertexInputState; }
    const auto& getInputAssemblyState() const noexcept { return m_inputAsemblyState; }
    const auto& getLayout() const noexcept { return m_pipelineLayout; }

private:
    const ve::LogicalDevice& m_logicalDevice;
    vk::PipelineDepthStencilStateCreateInfo m_depthStencilState{};
    vk::PipelineRasterizationStateCreateInfo m_rasterizerState{};
    vk::PipelineColorBlendStateCreateInfo m_colorBlendsState{};
    vk::PipelineMultisampleStateCreateInfo m_multisamplingState{};
    vk::PipelineViewportStateCreateInfo m_viewportState{};
    std::vector< vk::PipelineShaderStageCreateInfo > m_shaderStages;
    vk::PipelineDynamicStateCreateInfo m_dynamicState{};
    vk::PipelineVertexInputStateCreateInfo m_vertexInputState{};
    vk::PipelineInputAssemblyStateCreateInfo m_inputAsemblyState{};
    std::optional< ve::PipelineLayout > m_pipelineLayout{};

    void addShaderStage( const vk::ShaderStageFlagBits shaderType, const ve::ShaderModule& shaderModule );
    vk::PipelineDynamicStateCreateInfo defaultDynamicStatesInfo() const noexcept;
    vk::PipelineViewportStateCreateInfo defaultViewportStateInfo() const noexcept;
    vk::PipelineVertexInputStateCreateInfo defaultVertexInputInfo() const noexcept;
    vk::PipelineInputAssemblyStateCreateInfo defaultInputAsemblyInfo() const noexcept;
    vk::PipelineRasterizationStateCreateInfo defaultRasterizerInfo() const noexcept;
    vk::PipelineMultisampleStateCreateInfo defaultMultisamplingInfo() const noexcept;
    vk::PipelineColorBlendAttachmentState defaultColorBlendAttachmentState() const noexcept;
    vk::PipelineColorBlendStateCreateInfo defaultColorBlendStateInfo() const noexcept;
    vk::PipelineDepthStencilStateCreateInfo defaultDepthStencilInfo() const noexcept;
};

}; // namespace ve
