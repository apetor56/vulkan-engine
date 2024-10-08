#include "Image.hpp"

namespace ve {

Image::Image( const ve::MemoryAllocator& allocator, const vk::Extent2D extent, const vk::Format format,
              const vk::ImageUsageFlags usage, const vk::ImageTiling tiling )
    : m_memoryAllocator{ allocator }, m_imageExtent{ extent }, m_imageFormat{ format } {
    createImage( extent, format, usage, tiling );
}

Image::~Image() {
    vmaDestroyImage( m_memoryAllocator, m_image, m_allocation );
}

void Image::createImage( const vk::Extent2D extent, const vk::Format format, const vk::ImageUsageFlags usage,
                         const vk::ImageTiling tiling ) {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.sType         = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.extent.width  = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth  = 1U;
    imageInfo.mipLevels     = 1U;
    imageInfo.arrayLayers   = 1U;
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage         = usage;
    imageInfo.samples       = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage         = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VkMemoryPropertyFlags( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    vmaCreateImage( m_memoryAllocator, reinterpret_cast< VkImageCreateInfo * >( &imageInfo ), &allocationCreateInfo,
                    reinterpret_cast< VkImage * >( &m_image ), &m_allocation, nullptr );
}

} // namespace ve