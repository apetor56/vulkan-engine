#pragma once

#include "LogicalDevice.hpp"

namespace ve {

class RenderPass;

class Framebuffer : public utils::NonCopyable,
                    public utils::NonMovable {
public:
    using Attachments = std::array< vk::ImageView, 2U >;

    Framebuffer( const ve::RenderPass& renderPass, const Attachments& attachments, const vk::Extent2D extent );
    ~Framebuffer();

private:
    vk::Framebuffer m_framebuffer;
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve
