#include "Swapchain.hpp"
#include "QueueFamilyIndices.hpp"
#include "Config.hpp"

#include <algorithm>
#include <numeric>
#include <array>

namespace ve {

Swapchain::Swapchain( const ve::PhysicalDevice& physicalDevice, const ve::LogicalDevice& logicalDevice,
                      const ve::Window& window )
    : m_physicalDevice{ physicalDevice }, m_logicalDevice{ logicalDevice }, m_window{ window } {
    createSwapchain();
    createImageViews();
    createRenderPass();
    createFramebuffers();
}

Swapchain::~Swapchain() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    logicalDeviceHandler.destroySwapchainKHR( m_swapchain );

    std::ranges::for_each( m_swapchainImageViews, [ &logicalDeviceHandler ]( const auto& view ) {
        logicalDeviceHandler.destroyImageView( view );
    } );

    logicalDeviceHandler.destroyRenderPass( m_renderPass );

    std::ranges::for_each( m_framebuffers, [ &logicalDeviceHandler ]( const auto& framebuffer ) {
        logicalDeviceHandler.destroyFramebuffer( framebuffer );
    } );
}

void Swapchain::createSwapchain() {
    const auto physicalDeviceHandler{ m_physicalDevice.getHandler() };
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
    auto *const surface{ m_window.getSurface() };

    const Swapchain::Details swapchainDetails{ getSwapchainDetails( physicalDeviceHandler, surface ) };
    const vk::SurfaceFormatKHR surfaceFormat{ chooseSurfaceFormat( swapchainDetails.formats ) };
    const vk::PresentModeKHR presentationMode{ choosePresentationMode( swapchainDetails.presentationModes ) };
    const vk::Extent2D extent{ chooseExtent( swapchainDetails.capabilities ) };

    const std::uint32_t minImageCount{ swapchainDetails.capabilities.minImageCount };
    const std::uint32_t maxImageCount{ swapchainDetails.capabilities.maxImageCount };
    const std::uint32_t imageCount{ minImageCount != maxImageCount ? minImageCount + 1U : maxImageCount };

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = vk::StructureType::eSwapchainCreateInfoKHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1U;
    createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;

    const ve::QueueFamilyIndices indices{ QueueFamilyIndices::findQueueFamilies( physicalDeviceHandler, surface ) };
    const std::array< std::uint32_t, cfg::device::queueFamiliesCount > queueFamilyIndices{
        indices.graphicsFamilyID.value(), indices.presentationFamilyID.value() };

    if ( indices.graphicsFamilyID != indices.presentationFamilyID ) {
        createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = static_cast< std::uint32_t >( std::size( queueFamilyIndices ) );
        createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
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

    m_swapchain            = logicalDeviceHandler.createSwapchainKHR( createInfo );
    m_swapchainImages      = logicalDeviceHandler.getSwapchainImagesKHR( m_swapchain );
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainImageExtent = extent;
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
        return availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
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
            return availablePresentMode == vk::PresentModeKHR::eMailbox;
        } ) };
    if ( bestPresentationModeIt != std::end( availablePresentModes ) )
        return *bestPresentationModeIt;

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const noexcept {
    if ( capabilities.currentExtent.width != std::numeric_limits< std::uint32_t >::max() )
        return capabilities.currentExtent;

    int width{};
    int height{};
    glfwGetFramebufferSize( m_window.getHandler(), &width, &height );

    return { std::clamp( static_cast< uint32_t >( width ), capabilities.minImageExtent.width,
                         capabilities.maxImageExtent.width ),
             std::clamp( static_cast< uint32_t >( height ), capabilities.minImageExtent.height,
                         capabilities.maxImageExtent.height ) };
}

void Swapchain::createImageViews() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

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

    const auto createImageView{ [ this, logicalDeviceHandler, &createInfo ]( const auto image ) {
        createInfo.image = image;
        m_swapchainImageViews.emplace_back( logicalDeviceHandler.createImageView( createInfo ) );
    } };

    std::ranges::for_each( m_swapchainImages, createImageView );
}

void Swapchain::createRenderPass() {
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format         = m_swapchainImageFormat;
    colorAttachment.samples        = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0U;
    colorAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1U;
    subpass.pColorAttachments    = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = vk::StructureType::eRenderPassCreateInfo;
    renderPassInfo.attachmentCount = 1U;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1U;
    renderPassInfo.pSubpasses      = &subpass;

    vk::SubpassDependency dependency{};
    dependency.srcSubpass    = vk::SubpassExternal;
    dependency.dstSubpass    = 0U;
    dependency.srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    renderPassInfo.dependencyCount = 1U;
    renderPassInfo.pDependencies   = &dependency;

    m_renderPass = m_logicalDevice.getHandler().createRenderPass( renderPassInfo );
}

void Swapchain::createFramebuffers() {
    const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = vk::StructureType::eFramebufferCreateInfo;
    framebufferInfo.renderPass      = m_renderPass;
    framebufferInfo.attachmentCount = 1U;
    framebufferInfo.width           = m_swapchainImageExtent.width;
    framebufferInfo.height          = m_swapchainImageExtent.height;
    framebufferInfo.layers          = 1U;

    const auto createFramebuffer{ [ this, &framebufferInfo, logicalDeviceHandler ]( const vk::ImageView imageView ) {
        framebufferInfo.pAttachments = &imageView;
        m_framebuffers.emplace_back( logicalDeviceHandler.createFramebuffer( framebufferInfo ) );
    } };

    std::ranges::for_each( m_swapchainImageViews, createFramebuffer );
}

vk::Extent2D Swapchain::getExtent() const noexcept {
    return m_swapchainImageExtent;
}

vk::RenderPass Swapchain::getRenderpass() const noexcept {
    return m_renderPass;
}

vk::Framebuffer Swapchain::getFrambuffer( const std::uint32_t index ) const {
    return m_framebuffers.at( index );
}

std::uint32_t Swapchain::getImagesCount() const noexcept {
    return static_cast< std::uint32_t >( std::size( m_swapchainImages ) );
}

vk::SwapchainKHR Swapchain::getHandler() const noexcept {
    return m_swapchain;
}

} // namespace ve
