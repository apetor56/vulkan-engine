#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Vertex.hpp"
#include "Config.hpp"
#include "descriptor/DescriptorSetLayout.hpp"
#include "utils/Common.hpp"

#include <stdexcept>

namespace ve {

Pipeline::Pipeline( const PipelineBuilder& builder, const ve::RenderPass& renderPass )
    : m_logicalDevice{ renderPass.getLogicalDevice() } {
    const auto& pipelineLayout{ builder.getLayout() };
    const auto& shaderStages{ builder.getShaderStages() };

    if ( utils::size( shaderStages ) < 2U )
        throw std::runtime_error( "pipeline builder: shader stages are not set properly" );
    if ( !pipelineLayout.has_value() )
        throw std::runtime_error( "pipeline builder: pipeline layout is not set" );

    m_layout = pipelineLayout.value();

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = vk::StructureType::eGraphicsPipelineCreateInfo;
    pipelineInfo.stageCount          = utils::size( shaderStages );
    pipelineInfo.pStages             = std::data( shaderStages );
    pipelineInfo.pDynamicState       = &builder.getDynamicState();
    pipelineInfo.pViewportState      = &builder.getViewportState();
    pipelineInfo.pVertexInputState   = &builder.getVertexInputState();
    pipelineInfo.pInputAssemblyState = &builder.getInputAssemblyState();
    pipelineInfo.pRasterizationState = &builder.getRasterizerState();
    pipelineInfo.pMultisampleState   = &builder.getMultisamplingState();
    pipelineInfo.pColorBlendState    = &builder.getColorBlendState();
    pipelineInfo.pDepthStencilState  = &builder.getDepthStencilState();
    pipelineInfo.layout              = m_layout;
    pipelineInfo.renderPass          = renderPass.get();
    pipelineInfo.subpass             = 0U;

    auto [ result, pipeline ]{ m_logicalDevice.get().createGraphicsPipeline( nullptr, pipelineInfo ) };
    if ( result != vk::Result::eSuccess )
        throw std::runtime_error( "failed to create graphics pipeline" );

    m_pipeline = pipeline;
}

Pipeline::~Pipeline() {
    m_logicalDevice.get().destroyPipeline( m_pipeline );
}

PipelineLayout::PipelineLayout( const ve::LogicalDevice& logicalDevice, const vk::PipelineLayoutCreateInfo& layoutInfo )
    : m_logicalDevice{ logicalDevice } {
    m_pipelineLayout = m_logicalDevice.get().createPipelineLayout( layoutInfo );
}

vk::PipelineLayoutCreateInfo PipelineLayout::defaultInfo() noexcept {
    vk::PipelineLayoutCreateInfo info{};
    info.sType                  = vk::StructureType::ePipelineLayoutCreateInfo;
    info.pNext                  = nullptr;
    info.flags                  = vk::PipelineLayoutCreateFlags{};
    info.setLayoutCount         = 0U;
    info.pSetLayouts            = nullptr;
    info.pushConstantRangeCount = 0U;
    info.pPushConstantRanges    = nullptr;

    return info;
}

PipelineLayout::~PipelineLayout() {
    m_logicalDevice.get().destroyPipelineLayout( m_pipelineLayout );
}

PipelineBuilder::PipelineBuilder( const ve::LogicalDevice& logicalDevice )
    : m_dynamicState{ defaultDynamicStatesInfo() },
      m_viewportState{ defaultViewportStateInfo() },
      m_vertexInputState{ defaultVertexInputInfo() },
      m_inputAsemblyState{ defaultInputAsemblyInfo() },
      m_rasterizerState{ defaultRasterizerInfo() },
      m_multisamplingState{ defaultMultisamplingInfo() },
      m_colorBlendsState{ defaultColorBlendStateInfo() },
      m_depthStencilState{ defaultDepthStencilInfo() },
      m_colorBlendAttachmentState{ defaultColorBlendAttachmentState() },
      m_logicalDevice{ logicalDevice } {
    setSamplesCount( m_logicalDevice.getParentPhysicalDevice().getMaxSamplesCount() );
    setSampleShading( 1.0F );
}

PipelineBuilder::PipelineBuilder( const ve::LogicalDevice& logicalDevice, const ve::ShaderModule& vertexShader,
                                  const ve::ShaderModule& fragmentShader, const ve::PipelineLayout& pipelineLayout )
    : PipelineBuilder{ logicalDevice } {
    setShaders( vertexShader, fragmentShader );
    setLayout( pipelineLayout );
}

void PipelineBuilder::addShaderStage( const vk::ShaderStageFlagBits shaderType, const ve::ShaderModule& shaderModule ) {
    vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.sType  = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStageCreateInfo.stage  = shaderType;
    shaderStageCreateInfo.module = shaderModule.get();
    shaderStageCreateInfo.pName  = "main";

    m_shaderStages.emplace_back( shaderStageCreateInfo );
}

void PipelineBuilder::setShaders( const ve::ShaderModule& vertexShader, const ve::ShaderModule& fragmentShader ) {
    addShaderStage( vk::ShaderStageFlagBits::eVertex, vertexShader );
    addShaderStage( vk::ShaderStageFlagBits::eFragment, fragmentShader );
}

void PipelineBuilder::setLayout( const ve::PipelineLayout& pipelineLayout ) {
    m_pipelineLayout.emplace( pipelineLayout.get() );
}

void PipelineBuilder::setSamplesCount( const vk::SampleCountFlagBits samplesCount ) {
    m_multisamplingState.rasterizationSamples = samplesCount;
}

void PipelineBuilder::setSampleShading( const float minSampleShading ) {
    m_multisamplingState.sampleShadingEnable = vk::True;
    m_multisamplingState.minSampleShading    = minSampleShading;
}

[[nodiscard]] ve::Pipeline PipelineBuilder::build( const ve::RenderPass& renderPass ) {
    return ve::Pipeline{ *this, renderPass };
}

void PipelineBuilder::disableBlending() noexcept {
    m_colorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                 vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    m_colorBlendAttachmentState.blendEnable = vk::False;
}

void PipelineBuilder::enableBlendingAdditive() noexcept {
    m_colorBlendAttachmentState.blendEnable         = vk::True;
    m_colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    m_colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOne;
    m_colorBlendAttachmentState.colorBlendOp        = vk::BlendOp::eAdd;
    m_colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    m_colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    m_colorBlendAttachmentState.alphaBlendOp        = vk::BlendOp::eAdd;
    m_colorBlendAttachmentState.colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                 vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
}

void PipelineBuilder::disableDepthWrite() noexcept {
    m_depthStencilState.depthWriteEnable = vk::False;
}

vk::PipelineDynamicStateCreateInfo PipelineBuilder::defaultDynamicStatesInfo() const noexcept {
    static constexpr std::array< vk::DynamicState, 2U > dynamicStates{ vk::DynamicState::eViewport,
                                                                       vk::DynamicState::eScissor };

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamicState.dynamicStateCount = utils::size( dynamicStates );
    dynamicState.pDynamicStates    = std::data( dynamicStates );

    return dynamicState;
}

vk::PipelineViewportStateCreateInfo PipelineBuilder::defaultViewportStateInfo() const noexcept {
    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = vk::StructureType::ePipelineViewportStateCreateInfo;
    viewportState.pViewports    = nullptr;
    viewportState.pScissors     = nullptr;
    viewportState.viewportCount = 1U;
    viewportState.scissorCount  = 1U;

    return viewportState;
}

vk::PipelineVertexInputStateCreateInfo PipelineBuilder::defaultVertexInputInfo() const noexcept {
    vk::PipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType                           = vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertexInput.vertexBindingDescriptionCount   = 0U;
    vertexInput.pVertexBindingDescriptions      = nullptr;
    vertexInput.vertexAttributeDescriptionCount = 0U;
    vertexInput.pVertexAttributeDescriptions    = nullptr;

    return vertexInput;
}

vk::PipelineInputAssemblyStateCreateInfo PipelineBuilder::defaultInputAsemblyInfo() const noexcept {
    vk::PipelineInputAssemblyStateCreateInfo assembly{};
    assembly.sType                  = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    assembly.topology               = vk::PrimitiveTopology::eTriangleList;
    assembly.primitiveRestartEnable = vk::False;

    return assembly;
}

vk::PipelineRasterizationStateCreateInfo PipelineBuilder::defaultRasterizerInfo() const noexcept {
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

vk::PipelineMultisampleStateCreateInfo PipelineBuilder::defaultMultisamplingInfo() const noexcept {
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

vk::PipelineColorBlendAttachmentState PipelineBuilder::defaultColorBlendAttachmentState() const noexcept {
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

vk::PipelineColorBlendStateCreateInfo PipelineBuilder::defaultColorBlendStateInfo() const noexcept {
    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType               = vk::StructureType::ePipelineColorBlendStateCreateInfo;
    colorBlending.logicOpEnable       = vk::False;
    colorBlending.logicOp             = vk::LogicOp::eCopy;
    colorBlending.attachmentCount     = 1U;
    colorBlending.pAttachments        = &m_colorBlendAttachmentState;
    colorBlending.blendConstants[ 0 ] = 0.0F;
    colorBlending.blendConstants[ 1 ] = 0.0F;
    colorBlending.blendConstants[ 2 ] = 0.0F;
    colorBlending.blendConstants[ 3 ] = 0.0F;

    return colorBlending;
}

vk::PipelineDepthStencilStateCreateInfo PipelineBuilder::defaultDepthStencilInfo() const noexcept {
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

} // namespace ve
