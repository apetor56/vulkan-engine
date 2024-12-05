#include "Framebuffer.hpp"
#include "RenderPass.hpp"

#include "utils/Common.hpp"

namespace ve {

Framebuffer::Framebuffer( const ve::RenderPass& renderPass, const Attachments& attachments, const vk::Extent2D extent )
    : m_logicalDevice{ renderPass.getLogicalDevice() } {
    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = vk::StructureType::eFramebufferCreateInfo;
    framebufferInfo.renderPass      = renderPass.get();
    framebufferInfo.attachmentCount = utils::size( attachments );
    framebufferInfo.width           = extent.width;
    framebufferInfo.height          = extent.height;
    framebufferInfo.layers          = 1U;
    framebufferInfo.pAttachments    = std::data( attachments );

    m_framebuffer = m_logicalDevice.get().createFramebuffer( framebufferInfo );
}

Framebuffer::~Framebuffer() {
    m_logicalDevice.get().destroyFramebuffer( m_framebuffer );
}

} // namespace ve