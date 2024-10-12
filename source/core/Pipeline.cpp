#include "Pipeline.hpp"
#include "Vertex.hpp"
#include "Config.hpp"
#include "descriptor/DescriptorSetLayout.hpp"

#include <stdexcept>

namespace ve {
Pipeline::Pipeline( const ve::LogicalDevice& logicalDevice, const ve::Swapchain& swapchain,
                    const ve::DescriptorSetLayout& descriptorSetLayout )
    : m_vertexShader{ cfg::shader::vertShaderBinaryPath.string(), logicalDevice },
      m_fragmentShader{ cfg::shader::fragShaderBinaryPath.string(), logicalDevice },
      m_logicalDevice{ logicalDevice },
      m_swapchain{ swapchain } {
    createPipelineLayout( descriptorSetLayout );
    createPipeline();
}

Pipeline::~Pipeline() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    logicalDeviceHandler.destroyPipeline( m_graphicsPipeline );
    logicalDeviceHandler.destroyPipelineLayout( m_pipelineLayout );
}

void Pipeline::createPipeline() {
    const PipelineConfigInfo pipelineConfig{
        .dynamicState{ createDynamicStatesInfo() },
        .viewportState{ createViewportStateInfo() },
        .vertexInputState{ createVertexInputInfo() },
        .inputAsemblyState{ createInputAsemblyInfo() },
        .rasterizerState{ createRasterizerInfo() },
        .multisamplingState{ createMultisamplingInfo() },
        .colorBlendsState{ createColorBlendStateInfo( createColorBlendAttachmentState() ) },
        .depthStencilState{ createDepthStencilInfo() } };

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;

    const auto shaderStagesInfos{ createShaderStagesInfo() };
    pipelineInfo.stageCount = static_cast< std::uint32_t >( std::size( shaderStagesInfos ) );
    pipelineInfo.pStages    = std::data( shaderStagesInfos );

    pipelineInfo.pDynamicState       = &pipelineConfig.dynamicState;
    pipelineInfo.pViewportState      = &pipelineConfig.viewportState;
    pipelineInfo.pVertexInputState   = &pipelineConfig.vertexInputState;
    pipelineInfo.pInputAssemblyState = &pipelineConfig.inputAsemblyState;
    pipelineInfo.pRasterizationState = &pipelineConfig.rasterizerState;
    pipelineInfo.pMultisampleState   = &pipelineConfig.multisamplingState;
    pipelineInfo.pColorBlendState    = &pipelineConfig.colorBlendsState;
    pipelineInfo.pDepthStencilState  = &pipelineConfig.depthStencilState;

    pipelineInfo.layout     = m_pipelineLayout;
    pipelineInfo.renderPass = m_swapchain.getRenderpass();
    pipelineInfo.subpass    = 0U;

    auto [ result, pipeline ]{ m_logicalDevice.getHandler().createGraphicsPipeline( nullptr, pipelineInfo ) };
    if ( result != vk::Result::eSuccess )
        throw std::runtime_error( "failed to create graphics pipeline" );

    m_graphicsPipeline = pipeline;
}

vk::PipelineShaderStageCreateInfo Pipeline::pupulateShaderStageInfo( const vk::ShaderStageFlagBits shaderType,
                                                                     const ve::ShaderModule& shaderModule ) const {
    vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.sType  = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStageCreateInfo.stage  = shaderType;
    shaderStageCreateInfo.module = shaderModule.getHandler();
    shaderStageCreateInfo.pName  = "main";

    return shaderStageCreateInfo;
}

ShaderStageInfos Pipeline::createShaderStagesInfo() const {
    const auto vertStageInfo{ pupulateShaderStageInfo( vk::ShaderStageFlagBits::eVertex, m_vertexShader ) };
    const auto fragStageInfo{ pupulateShaderStageInfo( vk::ShaderStageFlagBits::eFragment, m_fragmentShader ) };

    return { vertStageInfo, fragStageInfo };
}

vk::PipelineDynamicStateCreateInfo Pipeline::createDynamicStatesInfo() const {
    static constexpr std::array< vk::DynamicState, 2U > dynamicStates{ vk::DynamicState::eViewport,
                                                                       vk::DynamicState::eScissor };

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamicState.dynamicStateCount = static_cast< std::uint32_t >( std::size( dynamicStates ) );
    dynamicState.pDynamicStates    = std::data( dynamicStates );

    return dynamicState;
}

vk::PipelineViewportStateCreateInfo Pipeline::createViewportStateInfo() const noexcept {
    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = vk::StructureType::ePipelineViewportStateCreateInfo;
    viewportState.pViewports    = nullptr;
    viewportState.pScissors     = nullptr;
    viewportState.viewportCount = 1U;
    viewportState.scissorCount  = 1U;

    return viewportState;
}

vk::PipelineVertexInputStateCreateInfo Pipeline::createVertexInputInfo() const noexcept {
    static constexpr auto vertexBindingDescription{ Vertex::getBindingDescription() };
    static const auto vertexAttributeDescriptions{ Vertex::getAttributeDescripstions() };

    vk::PipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType                         = vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertexInput.vertexBindingDescriptionCount = 1U;
    vertexInput.pVertexBindingDescriptions    = &vertexBindingDescription;
    vertexInput.vertexAttributeDescriptionCount =
        static_cast< std::uint32_t >( std::size( vertexAttributeDescriptions ) );
    vertexInput.pVertexAttributeDescriptions = std::data( vertexAttributeDescriptions );

    return vertexInput;
}

vk::PipelineInputAssemblyStateCreateInfo Pipeline::createInputAsemblyInfo() const noexcept {
    vk::PipelineInputAssemblyStateCreateInfo assembly{};
    assembly.sType                  = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    assembly.topology               = vk::PrimitiveTopology::eTriangleList;
    assembly.primitiveRestartEnable = vk::False;

    return assembly;
}

vk::PipelineRasterizationStateCreateInfo Pipeline::createRasterizerInfo() const noexcept {
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = vk::StructureType::ePipelineRasterizationStateCreateInfo;
    rasterizer.depthClampEnable        = vk::False;
    rasterizer.rasterizerDiscardEnable = vk::False;
    rasterizer.polygonMode             = vk::PolygonMode::eFill;
    rasterizer.lineWidth               = 1.0F;
    rasterizer.cullMode                = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace               = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable         = vk::False;

    return rasterizer;
}

vk::PipelineMultisampleStateCreateInfo Pipeline::createMultisamplingInfo() const noexcept {
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = vk::StructureType::ePipelineMultisampleStateCreateInfo;
    multisampling.sampleShadingEnable   = vk::False;
    multisampling.rasterizationSamples  = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading      = 1.0F;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = vk::False;
    multisampling.alphaToOneEnable      = vk::False;

    return multisampling;
}

vk::PipelineColorBlendAttachmentState Pipeline::createColorBlendAttachmentState() const noexcept {
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable         = vk::True;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttachment.colorBlendOp        = vk::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachment.alphaBlendOp        = vk::BlendOp::eAdd;
    colorBlendAttachment.colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    return colorBlendAttachment;
}

vk::PipelineColorBlendStateCreateInfo
    Pipeline::createColorBlendStateInfo( const vk::PipelineColorBlendAttachmentState state ) const noexcept {

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType               = vk::StructureType::ePipelineColorBlendStateCreateInfo;
    colorBlending.logicOpEnable       = vk::False;
    colorBlending.logicOp             = vk::LogicOp::eCopy;
    colorBlending.attachmentCount     = 1U;
    colorBlending.pAttachments        = &state;
    colorBlending.blendConstants[ 0 ] = 0.0F;
    colorBlending.blendConstants[ 1 ] = 0.0F;
    colorBlending.blendConstants[ 2 ] = 0.0F;
    colorBlending.blendConstants[ 3 ] = 0.0F;

    return colorBlending;
}

vk::PipelineDepthStencilStateCreateInfo Pipeline::createDepthStencilInfo() const noexcept {
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable       = vk::True;
    depthStencil.depthWriteEnable      = vk::True;
    depthStencil.depthCompareOp        = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = vk::False;
    depthStencil.minDepthBounds        = 0.0F;
    depthStencil.maxDepthBounds        = 1.0F;
    depthStencil.stencilTestEnable     = vk::False;

    return depthStencil;
}

void Pipeline::createPipelineLayout( const ve::DescriptorSetLayout& descriptorLayout ) {
    const auto layoutHandler{ descriptorLayout.getHandler() };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = vk::StructureType::ePipelineLayoutCreateInfo;
    pipelineLayoutInfo.setLayoutCount = 1U;
    pipelineLayoutInfo.pSetLayouts    = &layoutHandler;

    m_pipelineLayout = m_logicalDevice.getHandler().createPipelineLayout( pipelineLayoutInfo );
}

vk::Pipeline Pipeline::getHandler() const noexcept {
    return m_graphicsPipeline;
}

vk::PipelineLayout Pipeline::getLayout() const noexcept {
    return m_pipelineLayout;
}

} // namespace ve
