#pragma once

#include "LogicalDevice.hpp"

namespace ve {

class RenderPass;

class Framebuffer : public utils::NonCopyable {
public:
    using Attachments = std::array< vk::ImageView, 3U >;

    Framebuffer( const ve::RenderPass& renderPass, const Attachments& attachments, const vk::Extent2D extent );
    ~Framebuffer();

    Framebuffer( Framebuffer&& other ) noexcept;
    Framebuffer& operator=( Framebuffer&& other ) = delete;

    vk::Framebuffer get() const noexcept { return m_framebuffer; }

private:
    vk::Framebuffer m_framebuffer;
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve
