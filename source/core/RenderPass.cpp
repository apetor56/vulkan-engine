#include "RenderPass.hpp"

#include "utils/Common.hpp"

namespace ve {

RenderPass::RenderPass( const ve::LogicalDevice& logicalDevice, const vk::Format colorFormat,
                        const vk::Format depthFormat )
    : m_logicalDevice{ logicalDevice } {
    createDescription( colorFormat, vk::ImageLayout::ePresentSrcKHR );
    createDescription( depthFormat, vk::ImageLayout::eDepthStencilAttachmentOptimal );

    createReference( 0U, vk::ImageLayout::eColorAttachmentOptimal );
    createReference( 1U, vk::ImageLayout::eDepthStencilAttachmentOptimal );

    createSubpass();
    createSubpassDependency();
    createRenderPass();
}

RenderPass::~RenderPass() {
    m_logicalDevice.get().destroyRenderPass( m_renderPass );
}

void RenderPass::createDescription( const vk::Format format, const vk::ImageLayout finalLayout ) {
    m_attachmentDescriptions.emplace_back( vk::AttachmentDescriptionFlags{}, format, vk::SampleCountFlagBits::e1,
                                           vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                           vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                           vk::ImageLayout::eUndefined, finalLayout );
}

void RenderPass::createReference( const std::uint32_t attachmentIndex, const vk::ImageLayout layout ) {
    m_attachmentReferences.emplace_back( attachmentIndex, layout );
}

void RenderPass::createSubpass() {
    const auto& colorAttachmentRef{ m_attachmentReferences.at( 0 ) };
    const auto& depthAttachmentRef{ m_attachmentReferences.at( 1 ) };

    m_subpass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
    m_subpass.colorAttachmentCount    = 1U;
    m_subpass.pColorAttachments       = &colorAttachmentRef;
    m_subpass.pDepthStencilAttachment = &depthAttachmentRef;
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
