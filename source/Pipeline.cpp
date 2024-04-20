#include "Pipeline.hpp"
#include "Config.hpp"

namespace ve {
Pipeline::Pipeline( std::shared_ptr< LogicalDevice > logicalDevice, std::shared_ptr< Swapchain > swapchain )
    : m_vertexShader{ cfg::shader::vertShaderBinaryPath, logicalDevice },
      m_fragmentShader{ cfg::shader::fragShaderBinaryPath, logicalDevice },
      m_logicalDevice{ logicalDevice },
      m_swapchain{ swapchain } {
    createPipelineLayout();
    createViewport();
    createScissor();
    createPipeline();
}

Pipeline::~Pipeline() {
    vkDestroyPipeline( m_logicalDevice->getHandler(), m_graphicsPipeline, nullptr );
    vkDestroyPipelineLayout( m_logicalDevice->getHandler(), m_pipelineLayout, nullptr );
}

void Pipeline::createPipeline() {
    const PipelineConfigInfo pipelineConfig{ .dynamicState{ createDynamicStatesInfo() },
                                             .viewport{ createViewportStateInfo() },
                                             .vertexInput{ createVertexInputInfo() },
                                             .inputAsembly{ createInputAsemblyInfo() },
                                             .rasterizer{ createRasterizerInfo() },
                                             .multisampling{ createMultisamplingInfo() },
                                             .colorBlends{ createColorBlendAttachmentInfo(
                                                 createColorBlendAttachmentState() ) } };

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages    = createShaderStagesInfo().data();

    pipelineInfo.pDynamicState       = &pipelineConfig.dynamicState;
    pipelineInfo.pViewportState      = &pipelineConfig.viewport;
    pipelineInfo.pVertexInputState   = &pipelineConfig.vertexInput;
    pipelineInfo.pInputAssemblyState = &pipelineConfig.inputAsembly;
    pipelineInfo.pRasterizationState = &pipelineConfig.rasterizer;
    pipelineInfo.pMultisampleState   = &pipelineConfig.multisampling;
    pipelineInfo.pColorBlendState    = &pipelineConfig.colorBlends;
    pipelineInfo.pDepthStencilState  = nullptr;

    pipelineInfo.layout     = m_pipelineLayout;
    pipelineInfo.renderPass = m_swapchain->getRenderpass();
    pipelineInfo.subpass    = 0u;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;

    constexpr uint32_t graphicsPipelineInfosCount{ 1u };
    if ( vkCreateGraphicsPipelines( m_logicalDevice->getHandler(), VK_NULL_HANDLE, graphicsPipelineInfosCount,
                                    &pipelineInfo, nullptr, &m_graphicsPipeline ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to create graphics pipeline" );
    }
}

VkPipelineShaderStageCreateInfo Pipeline::pupulateShaderStageInfo( enum VkShaderStageFlagBits shaderType,
                                                                   const Shader& shader ) const {
    VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
    vertexStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageCreateInfo.stage  = shaderType;
    vertexStageCreateInfo.module = shader.getModule();
    vertexStageCreateInfo.pName  = "main";

    return vertexStageCreateInfo;
}

ShaderStageInfos Pipeline::createShaderStagesInfo() const {
    const auto vertStageInfo{ pupulateShaderStageInfo( VK_SHADER_STAGE_VERTEX_BIT, m_vertexShader ) };
    const auto fragStageInfo{ pupulateShaderStageInfo( VK_SHADER_STAGE_FRAGMENT_BIT, m_fragmentShader ) };

    return { vertStageInfo, fragStageInfo };
}

VkPipelineDynamicStateCreateInfo Pipeline::createDynamicStatesInfo() const {
    static constexpr std::array< VkDynamicState, 2u > dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT,
                                                                     VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast< uint32_t >( std::size( dynamicStates ) );
    dynamicState.pDynamicStates    = dynamicStates.data();

    return dynamicState;
}

VkPipelineViewportStateCreateInfo Pipeline::createViewportStateInfo() const {
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1u;
    viewportState.scissorCount  = 1u;

    return viewportState;
}

VkPipelineVertexInputStateCreateInfo Pipeline::createVertexInputInfo() const {
    VkPipelineVertexInputStateCreateInfo createInfo{};
    createInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.vertexBindingDescriptionCount   = 0;
    createInfo.pVertexBindingDescriptions      = nullptr;
    createInfo.vertexAttributeDescriptionCount = 0;
    createInfo.pVertexAttributeDescriptions    = nullptr;

    return createInfo;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::createInputAsemblyInfo() const {
    VkPipelineInputAssemblyStateCreateInfo createInfo{};
    createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    createInfo.primitiveRestartEnable = VK_FALSE;

    return createInfo;
}

void Pipeline::createViewport() {
    const auto& extent{ m_swapchain->getExtent() };
    m_viewport.x        = 0.0f;
    m_viewport.y        = 0.0f;
    m_viewport.width    = static_cast< float >( extent.width );
    m_viewport.height   = static_cast< float >( extent.height );
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;
}

void Pipeline::createScissor() {
    m_scissor.offset = { 0, 0 };
    m_scissor.extent = m_swapchain->getExtent();
}

VkPipelineRasterizationStateCreateInfo Pipeline::createRasterizerInfo() const {
    VkPipelineRasterizationStateCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.depthClampEnable        = VK_FALSE;
    createInfo.rasterizerDiscardEnable = VK_FALSE;
    createInfo.polygonMode             = VK_POLYGON_MODE_FILL;
    createInfo.lineWidth               = 1.0f;
    createInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
    createInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    createInfo.depthBiasEnable         = VK_FALSE;
    createInfo.depthBiasConstantFactor = 0.0f;
    createInfo.depthBiasClamp          = 0.0f;
    createInfo.depthBiasSlopeFactor    = 0.0f;

    return createInfo;
}

VkPipelineMultisampleStateCreateInfo Pipeline::createMultisamplingInfo() const {
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    return multisampling;
}

VkPipelineColorBlendAttachmentState Pipeline::createColorBlendAttachmentState() const {
    return { .blendEnable         = VK_TRUE,
             .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
             .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
             .colorBlendOp        = VK_BLEND_OP_ADD,
             .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
             .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
             .alphaBlendOp        = VK_BLEND_OP_ADD,
             .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                               VK_COLOR_COMPONENT_A_BIT };
}

VkPipelineColorBlendStateCreateInfo
    Pipeline::createColorBlendAttachmentInfo( VkPipelineColorBlendAttachmentState state ) const {

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable       = VK_FALSE;
    colorBlending.logicOp             = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount     = 1u;
    colorBlending.pAttachments        = &state;
    colorBlending.blendConstants[ 0 ] = 0.0f;
    colorBlending.blendConstants[ 1 ] = 0.0f;
    colorBlending.blendConstants[ 2 ] = 0.0f;
    colorBlending.blendConstants[ 3 ] = 0.0f;

    return colorBlending;
}

void Pipeline::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;
    pipelineLayoutInfo.pSetLayouts            = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;

    if ( vkCreatePipelineLayout( m_logicalDevice->getHandler(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout ) !=
         VK_SUCCESS ) {
        throw std::runtime_error( "failed to create pipeline layout!" );
    }
}

VkPipeline Pipeline::getHandler() const {
    return m_graphicsPipeline;
}

VkViewport Pipeline::getViewport() const {
    return m_viewport;
}

VkRect2D Pipeline::getScissor() const {
    return m_scissor;
}

} // namespace ve