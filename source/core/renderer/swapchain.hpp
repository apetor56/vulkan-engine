#pragma once

#include "LogicalDevice.hpp"
#include "Window.hpp"

#include "utils/Common.hpp"

#include <vector>
#include <optional>

namespace ve {

class Swapchain : public utils::NonCopyable,
                  public utils::NonMovable {
public:
    struct Details {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector< vk::SurfaceFormatKHR > formats;
        std::vector< vk::PresentModeKHR > presentationModes;
    };

    Swapchain( const ve::LogicalDevice& logicalDevice, ve::Window& window );
    ~Swapchain();

    void recreate();

    static Details getSwapchainDetails( const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface );
    vk::Extent2D getExtent() const noexcept { return m_swapchainImageExtent; }
    vk::SwapchainKHR get() const noexcept { return m_swapchain; }
    const std::vector< vk::ImageView >& getImageViews() const noexcept { return m_swapchainImageViews; }
    vk::Image getImage( const uint32_t imageIndex ) const { return m_swapchainImages.at( imageIndex ); }
    vk::ImageView getImageView( const uint32_t imageIndex ) const { return m_swapchainImageViews.at( imageIndex ); }
    vk::Viewport getViewport() const noexcept { return m_viewport; }
    vk::Rect2D getScissor() const noexcept { return m_scissor; }
    vk::Format getFormat() const noexcept { return m_swapchainImageFormat; }

private:
    std::vector< vk::Image > m_swapchainImages;
    std::vector< vk::ImageView > m_swapchainImageViews;

    vk::Viewport m_viewport;
    vk::Rect2D m_scissor;

    const ve::LogicalDevice& m_logicalDevice;
    ve::Window& m_window;

    vk::SwapchainKHR m_swapchain;
    vk::Extent2D m_swapchainImageExtent;
    vk::Format m_swapchainImageFormat;

    void createSwapchain();
    void createImageViews();
    void createViewport() noexcept;
    void createScissor() noexcept;

    void cleanup();

    vk::SurfaceFormatKHR
        chooseSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats ) const noexcept;
    vk::PresentModeKHR
        choosePresentationMode( const std::vector< vk::PresentModeKHR >& availablePresentModes ) const noexcept;
    vk::Extent2D chooseExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const noexcept;
};

} // namespace ve
