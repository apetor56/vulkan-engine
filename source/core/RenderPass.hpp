#pragma once

#include "LogicalDevice.hpp"

namespace ve {

class RenderPass : public utils::NonCopyable,
                   public utils::NonMovable {
public:
    RenderPass( const ve::LogicalDevice& logicalDevice, const vk::Format colorFormat, const vk::Format depthFormat );
    ~RenderPass();

    vk::RenderPass get() const noexcept { return m_renderPass; }
    const ve::LogicalDevice& getLogicalDevice() const noexcept { return m_logicalDevice; }

private:
    std::vector< vk::AttachmentDescription > m_attachmentDescriptions;
    std::vector< vk::AttachmentReference > m_attachmentReferences;
    vk::SubpassDescription m_subpass;
    vk::SubpassDependency m_subpassDependency;
    vk::RenderPass m_renderPass;
    const ve::LogicalDevice& m_logicalDevice;

    void createDescription( const vk::Format format, const vk::ImageLayout finalLayout );
    void createReference( const uint32_t attachmentIndex, const vk::ImageLayout layout );
    void createSubpass();
    void createSubpassDependency();
    void createRenderPass();
};

} // namespace ve
