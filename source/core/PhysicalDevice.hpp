#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "QueueFamilyIDs.hpp"

namespace ve {

class PhysicalDevice {
public:
    PhysicalDevice( const ve::VulkanInstance& instance, const ve::Window& window );

    PhysicalDevice( const PhysicalDevice& other ) = delete;
    PhysicalDevice( PhysicalDevice&& other )      = delete;

    PhysicalDevice& operator=( const PhysicalDevice& other ) = delete;
    PhysicalDevice& operator=( PhysicalDevice&& other )      = delete;

    vk::PhysicalDevice getHandler() const noexcept;
    const ve::extentions& getExtensions() const noexcept;
    [[nodiscard]] std::unordered_map< ve::FamilyType, std::uint32_t > getQueueFamilyIDs() const noexcept;

private:
    vk::PhysicalDevice m_physicalDevice{};
    ve::QueueFamilyIDs m_queueFamilies{};
    const ve::extentions m_deviceExtensions{ vk::KHRSwapchainExtensionName };

    const ve::VulkanInstance& m_instance;
    const ve::Window& m_window;

    void pickPhysicalDevice();
    std::uint32_t rate( const vk::PhysicalDevice device ) const;
    bool areRequiredExtensionsSupported( const vk::PhysicalDevice physicalDevice ) const;
};

} // namespace ve
