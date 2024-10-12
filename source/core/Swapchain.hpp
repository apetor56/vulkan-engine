#pragma once

#include "LogicalDevice.hpp"
#include "MemoryAllocator.hpp"
#include "Image.hpp"

#include <vector>
#include <optional>

namespace ve {

class Swapchain {
public:
    struct Details {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector< vk::SurfaceFormatKHR > formats;
        std::vector< vk::PresentModeKHR > presentationModes;
    };

    Swapchain( const ve::PhysicalDevice& physicalDevice, const ve::LogicalDevice& logicalDevice, ve::Window& window,
               const ve::MemoryAllocator& allocator );

    ~Swapchain();

    Swapchain( const Swapchain& other ) = delete;
    Swapchain( Swapchain&& other )      = delete;

    Swapchain& operator=( const Swapchain& other ) = delete;
    Swapchain& operator=( Swapchain&& other )      = delete;

    void recreate();

    static Details getSwapchainDetails( const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface );
    vk::Extent2D getExtent() const noexcept;
    vk::RenderPass getRenderpass() const noexcept;
    vk::Framebuffer getFrambuffer( const uint32_t index ) const;
    std::uint32_t getImagesCount() const noexcept;
    vk::SwapchainKHR getHandler() const noexcept;
    vk::Viewport getViewport() const noexcept;
    vk::Rect2D getScissor() const noexcept;

private:
    std::vector< vk::Image > m_swapchainImages;
    std::vector< vk::ImageView > m_swapchainImageViews;
    std::vector< vk::Framebuffer > m_framebuffers;

    vk::Viewport m_viewport;
    vk::Rect2D m_scissor;

    const ve::PhysicalDevice& m_physicalDevice;
    const ve::LogicalDevice& m_logicalDevice;
    ve::Window& m_window;
    const ve::MemoryAllocator& m_memoryAllocator;
    std::optional< ve::Image > m_depthImage;

    vk::SwapchainKHR m_swapchain;
    vk::Extent2D m_swapchainImageExtent;
    vk::RenderPass m_renderPass;
    vk::Format m_swapchainImageFormat;

    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
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
