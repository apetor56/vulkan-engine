#include "RenderPass.hpp"

#include "utils/Common.hpp"

namespace ve {

RenderPass::RenderPass( const ve::LogicalDevice& logicalDevice, const vk::Format colorFormat,
                        const vk::Format depthFormat, const vk::SampleCountFlagBits sampleCount )
    : m_logicalDevice{ logicalDevice } {
    createDescription( colorFormat, vk::ImageLayout::eColorAttachmentOptimal, sampleCount );
    createDescription( depthFormat, vk::ImageLayout::eDepthStencilAttachmentOptimal, sampleCount );
    createDescription( colorFormat, vk::ImageLayout::ePresentSrcKHR, vk::SampleCountFlagBits::e1,
                       vk::AttachmentLoadOp::eDontCare );

    createReference( 0U, vk::ImageLayout::eColorAttachmentOptimal );
    createReference( 1U, vk::ImageLayout::eDepthStencilAttachmentOptimal );
    createReference( 2U, vk::ImageLayout::eColorAttachmentOptimal );

    createSubpass();
    createSubpassDependency();
    createRenderPass();
}

RenderPass::~RenderPass() {
    m_logicalDevice.get().destroyRenderPass( m_renderPass );
}

void RenderPass::createDescription( const vk::Format format, const vk::ImageLayout finalLayout,
                                    const vk::SampleCountFlagBits sampleCount,
                                    const vk::AttachmentLoadOp loadOperation ) {
    m_attachmentDescriptions.emplace_back( vk::AttachmentDescriptionFlags{}, format, sampleCount, loadOperation,
                                           vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
                                           vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, finalLayout );
}

void RenderPass::createReference( const uint32_t attachmentIndex, const vk::ImageLayout layout ) {
    m_attachmentReferences.emplace_back( attachmentIndex, layout );
}

void RenderPass::createSubpass() {
    const auto& colorAttachmentRef{ m_attachmentReferences.at( 0U ) };
    const auto& depthAttachmentRef{ m_attachmentReferences.at( 1U ) };
    const auto& colorResolveAttachmentRef{ m_attachmentReferences.at( 2U ) };

    m_subpass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
    m_subpass.colorAttachmentCount    = 1U;
    m_subpass.pColorAttachments       = &colorAttachmentRef;
    m_subpass.pDepthStencilAttachment = &depthAttachmentRef;
    m_subpass.pResolveAttachments     = &colorResolveAttachmentRef;
}

void RenderPass::createSubpassDependency() {
    m_subpassDependency.srcSubpass   = vk::SubpassExternal;
    m_subpassDependency.dstSubpass   = 0U;
    m_subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    m_subpassDependency.srcStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    m_subpassDependency.dstStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    m_subpassDependency.dstAccessMask =
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
}

void RenderPass::createRenderPass() {
    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = vk::StructureType::eRenderPassCreateInfo;
    renderPassInfo.attachmentCount = utils::size( m_attachmentDescriptions );
    renderPassInfo.pAttachments    = std::data( m_attachmentDescriptions );
    renderPassInfo.subpassCount    = 1U;
    renderPassInfo.pSubpasses      = &m_subpass;
    renderPassInfo.dependencyCount = 1U;
    renderPassInfo.pDependencies   = &m_subpassDependency;

    m_renderPass = m_logicalDevice.get().createRenderPass( renderPassInfo );
}

} // namespace ve
