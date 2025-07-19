#pragma once

#include "MemoryAllocator.hpp"
#include "LogicalDevice.hpp"

#include <vulkan/vulkan.hpp>

namespace ve {

class Image : public utils::NonCopyable {
public:
    Image( const ve::MemoryAllocator& allocator, const ve::LogicalDevice& logicalDevice, const vk::Extent2D extent,
           const vk::Format format, const vk::ImageUsageFlags usage, const vk::ImageAspectFlagBits imageAspect,
           const uint32_t mipLevels = 1U, const vk::ImageTiling tiling = vk::ImageTiling::eOptimal );

    ~Image();

    Image( Image&& other ) noexcept;
    Image& operator=( Image&& other ) = delete;

    vk::Image get() const noexcept { return m_image; }
    vk::Extent2D getExtent() const noexcept { return m_imageExtent; }
    vk::ImageView getImageView() const noexcept { return m_imageView; }
    vk::Format getFormat() const noexcept { return m_imageFormat; }

private:
    vk::Image m_image{};
    vk::ImageView m_imageView{};
    VmaAllocation m_allocation{};
    const ve::MemoryAllocator& m_memoryAllocator;
    const ve::LogicalDevice& m_logicalDevice;
    const vk::Extent2D m_imageExtent{};
    const vk::Format m_imageFormat{};

    void createImage( const vk::ImageUsageFlags usage, const vk::ImageTiling tiling, const uint32_t mipLevels );
    void createImageView( const vk::ImageAspectFlagBits imageAspect, const uint32_t mipLevels );
};

} // namespace ve
