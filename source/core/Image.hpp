#pragma once

#include "MemoryAllocator.hpp"

#include <vulkan/vulkan.hpp>

namespace ve {

class Image {
public:
    Image( const ve::MemoryAllocator& allocator, const vk::Extent2D extent, const vk::Format format,
           const vk::ImageUsageFlags usage, const vk::ImageTiling tiling = vk::ImageTiling::eOptimal );

    ~Image();

    vk::Image get() const noexcept { return m_image; }
    vk::Extent2D getExtent() const noexcept { return m_imageExtent; }

private:
    vk::Image m_image{};
    vk::ImageView m_imageView{};
    VmaAllocation m_allocation{};
    const ve::MemoryAllocator& m_memoryAllocator;
    vk::Extent2D m_imageExtent{};
    vk::Format m_imageFormat{};

    void createImage( const vk::Extent2D extent, const vk::Format format, const vk::ImageUsageFlags usage,
                      const vk::ImageTiling tiling );
};

} // namespace ve