#include "Swapchain.hpp"
#include "QueueFamilyIDs.hpp"
#include "Config.hpp"

#include <algorithm>
#include <numeric>
#include <array>
#include <ranges>

namespace ve {

Swapchain::Swapchain( const ve::LogicalDevice& logicalDevice, ve::Window& window )
    : m_logicalDevice{ logicalDevice }, m_window{ window } {
    createSwapchain();
    createViewport();
    createScissor();
    createImageViews();
}

Swapchain::~Swapchain() {
    cleanup();
}

void Swapchain::createSwapchain() {
    const ve::PhysicalDevice& physicalDevice{ m_logicalDevice.getParentPhysicalDevice() };
    vk::Device logicalDeviceVk{ m_logicalDevice.get() };
    VkSurfaceKHR surface{ m_window.getSurface() };

    const Swapchain::Details swapchainDetails{ getSwapchainDetails( physicalDevice.get(), surface ) };
    const vk::SurfaceFormatKHR surfaceFormat{ chooseSurfaceFormat( swapchainDetails.formats ) };
    const vk::PresentModeKHR presentationMode{ choosePresentationMode( swapchainDetails.presentationModes ) };
    const vk::Extent2D extent{ chooseExtent( swapchainDetails.capabilities ) };

    const uint32_t minImageCount{ swapchainDetails.capabilities.minImageCount };
    const uint32_t maxImageCount{ swapchainDetails.capabilities.maxImageCount };
    const uint32_t imageCount{ minImageCount != maxImageCount ? minImageCount + 1U : maxImageCount };

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = vk::StructureType::eSwapchainCreateInfoKHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1U;
    createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;

    const auto& queueFamilyIDs{ physicalDevice.getQueueFamilyIDs() };

    if ( queueFamilyIDs.at( ve::FamilyType::eGraphics ) != queueFamilyIDs.at( ve::FamilyType::ePresentation ) ) {
        const auto valuesView{ queueFamilyIDs | std::views::values };
        const std::vector< uint32_t > indices{ std::begin( valuesView ), std::end( valuesView ) };
        createInfo.pQueueFamilyIndices   = std::data( indices );
        createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = static_cast< uint32_t >( std::size( indices ) );

    } else {
        createInfo.imageSharingMode      = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0U;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = swapchainDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode    = presentationMode;
    createInfo.clipped        = vk::True;
    createInfo.oldSwapchain   = nullptr;

    m_swapchain            = logicalDeviceVk.createSwapchainKHR( createInfo );
    m_swapchainImages      = logicalDeviceVk.getSwapchainImagesKHR( m_swapchain );
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainImageExtent = extent;
}

void Swapchain::recreate() {
    int width{};
    int height{};
    glfwGetFramebufferSize( m_window.get(), &width, &height );
    while ( width == 0 || height == 0 ) {
        if ( m_window.shouldClose() )
            return;

        glfwGetFramebufferSize( m_window.get(), &width, &height );
        glfwWaitEvents();
    }

    m_logicalDevice.get().waitIdle();

    cleanup();

    createSwapchain();
    createViewport();
    createScissor();
    createImageViews();
    m_window.setResizeFlag( false );
}

void Swapchain::cleanup() {
    const auto logicalDeviceVk{ m_logicalDevice.get() };

    std::ranges::for_each( m_swapchainImageViews,
                           [ &logicalDeviceVk ]( const auto& view ) { logicalDeviceVk.destroyImageView( view ); } );
    m_swapchainImageViews.clear();

    logicalDeviceVk.destroySwapchainKHR( m_swapchain );
}

Swapchain::Details Swapchain::getSwapchainDetails( const vk::PhysicalDevice physicalDevice,
                                                   const vk::SurfaceKHR surface ) {
    return { .capabilities{ physicalDevice.getSurfaceCapabilitiesKHR( surface ) },
             .formats{ physicalDevice.getSurfaceFormatsKHR( surface ) },
             .presentationModes{ physicalDevice.getSurfacePresentModesKHR( surface ) } };
}

vk::SurfaceFormatKHR
    Swapchain::chooseSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats ) const noexcept {
    const auto bestFormatIt{ std::ranges::find_if( availableFormats, []( const auto availableFormat ) {
        return availableFormat.format == vk::Format::eR8G8B8A8Srgb &&
               availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    } ) };
    if ( bestFormatIt != std::end( availableFormats ) )
        return *bestFormatIt;

    return availableFormats.at( 0U );
}

vk::PresentModeKHR
    Swapchain::choosePresentationMode( const std::vector< vk::PresentModeKHR >& availablePresentModes ) const noexcept {
    const auto bestPresentationModeIt{
        std::ranges::find_if( availablePresentModes, []( const auto availablePresentMode ) {
            return availablePresentMode == vk::PresentModeKHR::eFifo;
        } ) };
    if ( bestPresentationModeIt != std::end( availablePresentModes ) )
        return *bestPresentationModeIt;

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const noexcept {
    if ( capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max() )
        return capabilities.currentExtent;

    int width{};
    int height{};
    glfwGetFramebufferSize( m_window.get(), &width, &height );

    return { std::clamp( static_cast< uint32_t >( width ), capabilities.minImageExtent.width,
                         capabilities.maxImageExtent.width ),
             std::clamp( static_cast< uint32_t >( height ), capabilities.minImageExtent.height,
                         capabilities.maxImageExtent.height ) };
}

void Swapchain::createImageViews() {
    const auto logicalDeviceVk{ m_logicalDevice.get() };

    vk::ImageViewCreateInfo createInfo{};
    createInfo.sType                           = vk::StructureType::eImageViewCreateInfo;
    createInfo.viewType                        = vk::ImageViewType::e2D;
    createInfo.format                          = m_swapchainImageFormat;
    createInfo.components.r                    = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g                    = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b                    = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a                    = vk::ComponentSwizzle::eIdentity;
    createInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel   = 0U;
    createInfo.subresourceRange.levelCount     = 1U;
    createInfo.subresourceRange.baseArrayLayer = 0U;
    createInfo.subresourceRange.layerCount     = 1U;

    std::ranges::for_each( m_swapchainImages, [ this, logicalDeviceVk, &createInfo ]( const auto image ) {
        createInfo.image = image;
        m_swapchainImageViews.emplace_back( logicalDeviceVk.createImageView( createInfo ) );
    } );
}

void Swapchain::createViewport() noexcept {
    m_viewport.x        = 0.0F;
    m_viewport.y        = 0.0F;
    m_viewport.width    = static_cast< float >( m_swapchainImageExtent.width );
    m_viewport.height   = static_cast< float >( m_swapchainImageExtent.height );
    m_viewport.minDepth = 0.0F;
    m_viewport.maxDepth = 1.0F;
}

void Swapchain::createScissor() noexcept {
    m_scissor.offset = vk::Offset2D{ 0, 0 };
    m_scissor.extent = m_swapchainImageExtent;
}

} // namespace ve
