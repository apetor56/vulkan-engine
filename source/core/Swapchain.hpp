#pragma once

#include "LogicalDevice.hpp"

#include <vector>

namespace ve {

class Swapchain {
public:
    struct Details {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector< vk::SurfaceFormatKHR > formats;
        std::vector< vk::PresentModeKHR > presentationModes;
    };

    Swapchain( const ve::PhysicalDevice& physicalDevice, const ve::LogicalDevice& logicalDevice,
               const ve::Window& window );

    ~Swapchain();

    Swapchain( const Swapchain& other ) = delete;
    Swapchain( Swapchain&& other )      = delete;

    Swapchain& operator=( const Swapchain& other ) = delete;
    Swapchain& operator=( Swapchain&& other )      = delete;

    static Details getSwapchainDetails( const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface );

    vk::Extent2D getExtent() const noexcept;
    vk::RenderPass getRenderpass() const noexcept;
    vk::Framebuffer getFrambuffer( const uint32_t index ) const;
    std::uint32_t getImagesCount() const noexcept;
    vk::SwapchainKHR getHandler() const noexcept;

private:
    std::vector< vk::Image > m_swapchainImages;
    std::vector< vk::ImageView > m_swapchainImageViews;
    std::vector< vk::Framebuffer > m_framebuffers;

    const ve::PhysicalDevice& m_physicalDevice;
    const ve::LogicalDevice& m_logicalDevice;
    const ve::Window& m_window;

    vk::SwapchainKHR m_swapchain;
    vk::Extent2D m_swapchainImageExtent;
    vk::RenderPass m_renderPass;
    vk::Format m_swapchainImageFormat;

    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();

    vk::SurfaceFormatKHR
        chooseSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats ) const noexcept;
    vk::PresentModeKHR
        choosePresentationMode( const std::vector< vk::PresentModeKHR >& availablePresentModes ) const noexcept;
    vk::Extent2D chooseExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const noexcept;
};

} // namespace ve
