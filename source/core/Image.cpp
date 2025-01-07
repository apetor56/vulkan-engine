#include "Image.hpp"

namespace ve {

Image::Image( const ve::MemoryAllocator& allocator, const ve::LogicalDevice& logicalDevice, const vk::Extent2D extent,
              const vk::Format format, const vk::ImageUsageFlags usage, const vk::ImageAspectFlagBits imageAspect,
              const vk::ImageTiling tiling )
    : m_memoryAllocator{ allocator },
      m_logicalDevice{ logicalDevice },
      m_imageExtent{ extent },
      m_imageFormat{ format } {
    createImage( usage, tiling );
    createImageView( imageAspect );
}

Image::~Image() {
    m_logicalDevice.get().destroyImageView( m_imageView );
    vmaDestroyImage( m_memoryAllocator.get(), m_image, m_allocation );
}

void Image::createImage( const vk::ImageUsageFlags usage, const vk::ImageTiling tiling ) {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.sType         = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.extent.width  = m_imageExtent.width;
    imageInfo.extent.height = m_imageExtent.height;
    imageInfo.extent.depth  = 1U;
    imageInfo.mipLevels     = 1U;
    imageInfo.arrayLayers   = 1U;
    imageInfo.format        = m_imageFormat;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage         = usage;
    imageInfo.samples       = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage         = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VkMemoryPropertyFlags( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    vmaCreateImage( m_memoryAllocator.get(), reinterpret_cast< VkImageCreateInfo * >( &imageInfo ),
                    &allocationCreateInfo, reinterpret_cast< VkImage * >( &m_image ), &m_allocation, nullptr );
}

void Image::createImageView( const vk::ImageAspectFlagBits imageAspect ) {
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image                           = m_image;
    viewInfo.viewType                        = vk::ImageViewType::e2D;
    viewInfo.format                          = m_imageFormat;
    viewInfo.subresourceRange.aspectMask     = imageAspect;
    viewInfo.subresourceRange.baseMipLevel   = 0U;
    viewInfo.subresourceRange.levelCount     = 1U;
    viewInfo.subresourceRange.baseArrayLayer = 0U;
    viewInfo.subresourceRange.layerCount     = 1U;

    m_imageView = m_logicalDevice.get().createImageView( viewInfo );
}

} // namespace ve
